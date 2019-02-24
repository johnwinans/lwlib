<?php
//***************************************************************************
//
// This file is part of lwlib.
//
// Copyright (C) 2002, 2003 John Winans and Joseph M. Link
// 
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 2.1 of the License, or (at your option) any later version.
// 
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// Lesser General Public License for more details.
// 
// You should have received a copy of the GNU Lesser General Public
// License along with this library; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
// 
//***************************************************************************

define('LW_STRING', 1);
define('LW_FILENAME', 2);

class LW_Template
{
    /* 'yes'        halt,
     * 'report'     report error, continue,
     * 'no'         ignore error quietly
     */
    var $halt_on_error = 'report';

    /***** private ******/

    /* template source */
    var $_source = '';

    /* parsed output */
    var $_output = '';

    /* $_globals[key] = 'value';
     * contains global (persistent across parses) variables
     */
    var $_globals = array();

    /* $_locals[key] = 'value';
     * contains local variables (emptied after each parse)
     */
    var $_locals = array();

    /* last error message is retained here */
    var $_last_error = '';

    /* Include HTML comments showing where templates came from */
    var $_debug = 0;
    var $_origin = '';

    //*************************************************************************
    /**
    * Constructor
    *
    * $template = new LW_Template($file_name);
    *    Creates a new template object from the contents of the file $filename
    *
    * $template = new LW_Template($string, LW_STRING);
    *    Creates a new template object from the contents of the string $string
    *
    * $template = new LW_Template($filename, LW_FILENAME, $template_obj);
    *    Creates a new template object from the contents of the file $filename
    *    The new object's 'global' variables is initialized to the contents of
    *    the $template_obj's global AND local variables.
    *
    * $template = new LW_Template($string, LW_STRING, $template_obj);
    *    Creates a new template object from the contents of the string $string
    *    The new object's 'global' variables is initialized to the contents of
    *    the $template_obj's global AND local variables.
    */
    function LW_Template($arg1, $type = LW_FILENAME, $parent_obj = null)
    {
        if($type == LW_FILENAME)
        {
            $file_name = $arg1;

            if(file_exists($file_name))
                $this->_source = implode('', @file($file_name));

            if($this->_source === '')
            {
                $this->_halt(
                    "LW_Template(): While loading file $file_name: " .
                    'does not exist or is empty.');
            }
            $this->_origin = $file_name;
        }
        else if($type == LW_STRING)
        {
            $this->_source = $arg1;
            $this->_origin = '<string>';
        }
        else
            $this->_halt('LW_Template(): Invalid second argument');
        
        if($parent_obj)
        {
            $this->_globals = array_merge(
                $parent_obj->_globals, $parent_obj->_locals);
            $this->_origin = $parent_obj->_origin . '::' . $this->_origin;
        }
        
        if ($this->_debug)
        {
            $this->_source = "\n<!-- TEMPLATE BEGIN {$this->_origin} -->\n\n" .
            $this->_source . "\n<!-- TEMPLATE END {$this->_origin} -->\n";
        }
    }
    
    /**
     * Extract the first occurrence of the block named $block_name from this
     * template and, if $replace == true replace it with {$block_name}.
     * Return the contents of the block.
     */
    function extractBlock($block_name, $replace = true)
    {
        $pattern =
            "/<!-- BEGIN $block_name -->\n?(.*?)<!-- END $block_name -->\n?/s";
        preg_match($pattern, $this->_source, $matches);
        
        if($replace)
        {
            $this->_source = preg_replace(
                $pattern, '{' . $block_name . '}', $this->_source, 1);
        }
        
        if(!isset($matches[1]))
        {
            $this->_halt(
                "LW_Template(): While loading block $block_name:" .
                'does not exist.');
        }
        else
            return $matches[1];
    }

    /**
     * Set local template variables.  All local variables are unset after each
     * call to parse()
     *
     * set_var(array $values)
     *  values: array of variable name, value pairs.
     *
     * set_var(string $varname, string $value)
     *  varname: name of a variable that is to be defined
     *  value:   value of that variable
     */
    function setVar($varname, $value = '')
    {
        if(is_array($varname))
            $this->_locals = array_merge($this->_locals, $varname);
        else
            $this->_locals[$varname] = $value;
    }

    /**
     * Set global template variables
     *
     * set_global_var(array $values)
     *  values: array of variable name, value pairs.
     *
     * set_global_var(string $varname, string $value)
     *  varname: name of a variable that is to be defined
     *  value:   value of that variable
     */
    function setGlobalVar($varname, $value = '')
    {
        if(is_array($varname))
            $this->_globals = array_merge($this->_globals, $varname);
        else
            $this->_globals[$varname] = $value;
    }

    /**
     * Parse template and append to output string (initially empty)
     */
    function parse()
    {
        /* first parse all the conditionals */
        $pattern = "/<!-- IF (.*?) -->\n?(.*?)" .
                   "(?:<!-- ELSE \\1 -->\n?(.*?))?" .
                   "\<!-- ENDIF \\1 -->\n?/s";
        $str = preg_replace_callback(
            $pattern,
            array($this, '_parse_conditionals_cb'),
            $this->_source);

        /* now parse out all template variables */
        $this->_output .= preg_replace_callback(
            '/{([^ \t\r\n}]+)}/',
            array($this, '_parse_variables_cb'),
            $str);

        $this->_locals = array();
    }

    /**
     * Append arbitrary string to output string
     */
    function append($str)
    {
        $this->_output .= $str;
    }
    
    /**
     * Set output string
     */
    function set($str)
    {
        $this->_output = $str;
    }
    
    /**
     * get parsed output string
     */
    function get()
    {
        return $this->_output;
    }

    /**
     * Process any template syntax garbage left over.
     *
     * $vars = 'keep'       keep remaining variables
     * $vars = 'remove'     remove undefined variables
     * $vars = 'comment'    replace undefined variables with comments
     *
     * $blocks = 'remove'   remove remaining block descriptors
     * $blocks = 'keep'     keep remaining block descriptors
     */
    function finalize($vars = 'remove', $blocks = 'keep')
    {
        switch($vars)
        {
          case 'remove':
            $this->_output = preg_replace(
                '/{[^ \t\r\n}]+}/', '', $this->_output);
            break;

          case 'comment':
            $this->_output = preg_replace(
                '/{([^ \t\r\n}]+)}/',
                '<!-- LW_Template : Variable \1 undefined -->',
                $this->_output);
            break;

          default:
            break;
        }

        switch($blocks)
        {
          case 'remove':
            $patterns = array('/<!-- BEGIN .*? -->/', '/<!-- END .*? -->/');
            $this->_output = preg_replace($patterns, '', $this->_output);
            break;

          default:
            break;
        }
    }

    /*************************************************************************
     * Private methods
     */
    /**
     * private callback for preg_replace_callback(), used by parse()
     */
    function _parse_conditionals_cb($matches)
    {
        $block = '';

        if(isset($this->_locals[$matches[1]]) ||
            isset($this->_globals[$matches[1]]))
        {
            $block = $matches[2];       /* IF var */
        }
        else if(isset($matches[3]))
            $block = $matches[3];       /* ELSE */

        if($block !== '')
        {
            $pattern = "/<!-- IF (.*?) -->\n?(.*?)" .
                       "(?:<!-- ELSE \\1 -->\n?(.*?))?" .
                       "<!-- ENDIF \\1 -->\n?/s";
            return preg_replace_callback(
                $pattern, array($this, '_parse_conditionals_cb'), $block);
        }

        return '';                      /* no match */
    }

    /**
     * private callback for preg_replace_callback(), used by parse()
     */
    function _parse_variables_cb($matches)
    {
        $varname = $matches[1];

        if(isset($this->_locals[$varname]))
            return $this->_locals[$varname];
        else if(isset($this->_globals[$varname]))
            return $this->_globals[$varname];
        else
            return '{' . $varname . '}';
    }


    function _halt($message)
    {
        $this->_last_error = $message;

        if($this->halt_on_error != 'no')
            print "<b>LW_Template Error:</b> $message<br>\n";

        if($this->halt_on_error == 'yes')
            die('<b>Halted.</b>');
    }
}
?>
