# LW_Template

LW_Template is a PHP class that provides template services suitable for web site deployment.

## Implementation Model
A template is a regular PHP string that may originate from a file, be extracted from some other template, or from a string constant or variable.

Symbolic substitution may be performed on the template in a number of ways.

* Variable substitution
* Block extraction, replication and/or replacement
* Conditional elimination using an IF-ELSE-ENDIF structure 

The programming model chosen for LW_Template is that of having a single template class instance represent only one template string. If one extracts a block from a template with the intent to manipulate it as another template, a new template instance is created for it. The result is an intuitive and simple API and a clean, efficient implementation.

## Scope of Variables

LW_Template uses the notion of global and local variables. These are primairly important to those templates that are parsed more than one time (as is the normal case for repeating blocks).

Global variables remain set unless changed by the program using setGlobalVar().

Local variables are automatically cleared after the template is parsed. When building a repeating block, one would set local variables using setVar() in a loop that ends with a parse() operation. At the top of the loop, the locals would be cleared out so that no local variables could bleed into subsequent loop iterations (useful for conditionally set variables.)

When ever a new template instance is created from an existing one via block extraction, all global and local variables currently set in the source template are automatically set as global variables in the new/extracted template.

If ever a local and global variable has the same name, the local value takes precidence over that of the global.

## General Template Grammar Rules

### Variables
Variable names must match the following:

```
{[0-9A-Za-z_:-]+}
```

Examples of valid names include:

```
{NAME}
{Name}
{ERR:Name}
{THERE_ONE_WAS_A_MAN_FROM_NANTUCKET}
{_123456789}
```

Examples of bogosities include:

```
{A B} (Spaces are not allowed)
{A?B} (Invalid special character)
{ABO } (Space at the end of the name)
```

### Blocks

The purpose of a block is to provide a way to replicate a fragment of text (for example a table row) an arbitrary number of times, each time setting variables (and/or nested blocks) to possibly different values.

The format of a dynamic block named `ZEKE` is:

```
<!-- BEGIN ZEKE -->
Name: {NAME}
Address: {ADDR}
Email: {EMAIL}
<!-- END ZEKE -->
```

The spacing in the BEGIN and END tags are important. If more than one block with the SAME NAME exists in a single template, results will be indeterminate.
IF-ELSE Blocks

The purpose of an IF-ELSE block is to provide a simple way to programmatically turn on or off a fragment of text in the template.

The format of a IF-ELSE block named `ZEKE` is:

```
<!-- IF ZEKE -->

If the variable 'ZEKE' is defined when this template is parsed, this text will appear in the result.

Exactly what the variable is set to is not important and is ignored by the IF-ELSE logic. It can even appear elsewhere as a variable or as a block name.

<!-- ELSE ZEKE -->

If the variable 'ZEKE' is *not* defined when this template is parsed, this text will appear in the result.

Note that the ELSE portion of an IF-ELSE block is optional.

<!-- ENDIF ZEKE -->
```

The spacing within the IF, ELSE and ENDIF tags are important. Multiple IF-THEN blocks may exist in the same template with the same name. The tags need not, however appear on their own lines. They may all appear on one line, and/or intermixed with other text:

```
<!-- IF ZEKE -->ZEKE is {ZEKE}.
<!-- ELSE ZEKE -->There is no ZEKE defined.<!-- ENDIF ZEKE -->
```

# LW_Template API Details

## Public Variables

`$halt_on_error`

Default = 'report'

Valid values are:

* yes = halt using die()
* report = report error, continue
* no = silently ignore errors 

`$_debug`

This is not really a public variable, but is a useful hack when debugging complex templates. Setting it to a nonzero value will cause HTML comments to be inserted at the begining and end of each template along with information about where the template came from (filename or string).

## Public Constructors

`function LW_Template($arg1, $type = LW_FILENAME, $parent_obj = null)`

Using a File

`$template = new LW_Template($file_name);`

Creates a new template object from the contents of the file $filename

`$template = new LW_Template($filename, LW_FILENAME, $template_obj);`

Creates a new template object from the contents of the file $filename The new object's 'global' variables is initialized to the contents of the $template_obj's global AND local variables.
Using a String

`$template = new LW_Template($string, LW_STRING);`

Creates a new template object from the contents of the string $string

`$template = new LW_Template($string, LW_STRING, $template_obj);`

Creates a new template object from the contents of the string $string The new object's 'global' variables is initialized to the contents of the $template_obj's global AND local variables.

## Public Methods

### `function extractBlock($block_name, $replace = true)`

Extract the first occurrence of the block named $block_name from this template and, if $replace == true replace it with {$block_name}. Return the contents of the block.

### Combining the constructor with the extract

`$tr = new LW_Template($pt->extractBlock('ROW'), LW_STRING);`

This will extract the block named ROW from the template $pt and then create a new template $tr using the data from that block.

The extracted block within $pt is replaced with a template variable named ROW (because we did not tell it to supress the replacement) so that we can later do a $pt->setVar('ROW', ...); on it.

See the repeating block example code for more detail.

### `function setVar($varname, $value = '')`

Set local template variables. All local variables are unset after each call to parse(). This function may be used to set one variable or an array of values.

Example signatures:

#### `setVar(string $varname, string $value)`

Where $varname is the name of a variable that is to be defined and $value is the value of that variable.

#### `setVar(array $values)`

Where $values is an array of variable name, value pairs.

### `function setGlobalVar($varname, $value = '')`

Set global template variables.

Example signatures:

#### `setGlobalVar(array $values)`

Where $values is an array of variable name, value pairs.

#### `setGlobalVar(string $varname, string $value)`

Where $varname is the name of a variable that is to be defined and $value is the value of that variable.

### `function parse()`

Parse template and append it to output buffer/string (initially empty).

### `function set($str)`

Set the current value of the output buffer/string.

### `function append($str)`

Append arbitrary string to output buffer/string.

### `function get()`

Return the current value of the output buffer/string.

### `function finalize($vars='remove', $blocks='keep')`

Removes or comments out left over variables and/or blocks from the current output buffer.

Valid values for parameters:

#### `$vars`

Default = 'remove'

Valid values are:

* 'remove' = remove undefined variables
* 'comment' = replace undefined variables with comments
* 'keep' = keep undefined variables 

#### `$blocks`

Default = 'keep'

Valid values are:

* 'remove' = remove remaining block descriptors
* 'keep' = keep remaining block descriptors 

# Programming Examples

## A Template With Variables

The Template:

```
<html>
  <head>
    <title>{TITLE}</title>
  </head>
  <body bgcolor='{BGCOLOR}'>
    Hello {NAME},  Welcome to the template demo.
  </body>
</html>
```

The Code:

```
require_once('LW_Template.inc');

$t = new LW_Template('x1.lwt');
$t->setVar('TITLE', 'The title of the page');
$t->setVar('BGCOLOR', '#c0ffff');
$t->setVar('NAME', 'John');
$t->parse();
$t->finalize();
echo $t->get();
```

The use of `$t->finalize()` in this example will have no effect since there are no variables or blocks that are left unset. It is present only to illustrate where it should otherwise occur.

The Result:

```
<html>
  <head>
    <title>The title of the page</title>
  </head>
  <body bgcolor='#c0ffff'>
    Hello John,  Welcome to the template demo.
  </body>
</html>
```


## A Repeating Block

The Template:

```
<html>
  <head>
    <title>{TITLE}</title>
  </head>
  <body <!-- IF BGCOLOR -->bgcolor='{BGCOLOR}'<!-- ENDIF BGCOLOR -->>
    <p>
    Hello {NAME},  Welcome to the template demo.
    </p>
    <p>
    <table border='1'>
      <tr>
        <th>col1</th><th>col2</th><th>col3</th>
      </tr>
<!-- BEGIN ROW -->
        <tr><td>{COL1}</td><td>{COL2}</td><td>{COL3}</td></tr>
<!-- END ROW -->
    </table>
  </body>
</html>
```

Note that by placing block tags (BEGIN, END, IF, ELSE, ENDIF) that appear on lines by themselves at the far-left of the line, the parsed output indentation will cleanly match that of the template.

The Code:

```
require_once('LW_Template.inc');

$t = new LW_Template('x2.lwt');
$t->setVar('TITLE', 'The title of the page');
$t->setVar('NAME', 'John');

$r = new LW_Template($t->extractBlock('ROW'), LW_STRING);

for ($i=0; $i<3; $i++)
{
        $r->setVar('COL1', "col1-$i");
        $r->setVar('COL2', "col2-$i");
        $r->setVar('COL3', "col3-$i");
        $r->parse();
}
$t->setVar('ROW', $r->get());

$t->parse();
$t->finalize();
echo $t->get();
```

The use of $t->finalize() in this example will have no effect since there are no variables or blocks that are left unset. It is present only to illustrate where it would normally occur.
The Result:

```
<html>
  <head>
    <title>The title of the page</title>
  </head>
  <body >
    <p>
    Hello John,  Welcome to the template demo.
    </p>
    <p>
    <table border='1'>
      <tr>
        <th>col1</th><th>col2</th><th>col3</th>
      </tr>
        <tr><td>col1-0</td><td>col2-0</td><td>col3-0</td></tr>
        <tr><td>col1-1</td><td>col2-1</td><td>col3-1</td></tr>
        <tr><td>col1-2</td><td>col2-2</td><td>col3-2</td></tr>
    </table>
  </body>
</html>
```

Note that the variable BGCOLOR was not set, so the if-block was eliminated.
Last updated: 2003-01-30


## Nested Blocks

The Template:

```
<html>
  <head>
    <title>{TITLE}</title>
  </head>
  <body <!-- IF BGCOLOR -->bgcolor='{BGCOLOR}'<!-- ENDIF BGCOLOR -->>
    <p>
      Hello {NAME},  Welcome to the template demo.
    </p>
    <p>
<!-- IF T1ROW -->
      <table border='1'>
        <tr>
          <th>col1</th><th>col2</th>
        </tr>
<!-- BEGIN T1ROW --><tr><td>{COL1}</td><td>{COL2}</td></tr><!-- END T1ROW -->
      </table>
<!-- ELSE T1ROW -->
        There aint no rows in table 1.
<!-- ENDIF T1ROW -->
    </p>

    <p>
<!-- IF T2ROW -->
        <table border='1'>
<!-- BEGIN T2ROW -->
            <tr><!-- BEGIN T2COL --><td>{COL} [{glob} {locl}]</td><!-- END T2COL --></tr>
<!-- END T2ROW -->
        </table>
<!-- ELSE T2ROW -->
        There aint no rows in table 2.
<!-- ENDIF T2ROW -->
    </p>
  </body>
</html>
```

The Code:

```
require_once('LW_Template.inc');

$t = new LW_Template('x3.lwt');
$t->setVar('TITLE', 'The title of the page');
$t->setVar('NAME', 'John');

// extract the outter block
$row = new LW_Template($t->extractBlock('T2ROW'), LW_STRING);


$row->setVar('glob', 'G');              // 'glob' is a local var in $row


// extract the inner block ($row as parm3 means to copy its vars)
$col = new LW_Template($row->extractBlock('T2COL'), LW_STRING, $row);

// 'glob' now a global var in $col

for ($r=0; $r<4; $r++)
{
  $col->setVar('locl', "L-$r");         // remains set only for first loop pass
  $col->set('');                        // clear the output buffer
  for ($c=0; $c<3; $c++)
  {
    $col->setVar('COL', "($r, $c)");    // 'COL' is a local var in $col
    $col->parse();                      // will clear the locals in $col
  }
  $row->setVar('T2COL', $col->get());
  $row->parse();
}
$t->setVar('T2ROW', $row->get());

$t->parse();
$t->finalize();
echo $t->get();
```

The Result:

```
<html>
  <head>
    <title>The title of the page</title>
  </head>
  <body >
    <p>
      Hello John,  Welcome to the template demo.
    </p>
    <p>
        There aint no rows in table 1.
    </p>

    <p>
        <table border='1'>
            <tr><td>(0, 0) [G L-0]</td><td>(0, 1) [G ]</td><td>(0, 2) [G ]</td></tr>
            <tr><td>(1, 0) [G L-1]</td><td>(1, 1) [G ]</td><td>(1, 2) [G ]</td></tr>
            <tr><td>(2, 0) [G L-2]</td><td>(2, 1) [G ]</td><td>(2, 2) [G ]</td></tr>
            <tr><td>(3, 0) [G L-3]</td><td>(3, 1) [G ]</td><td>(3, 2) [G ]</td></tr>
        </table>
    </p>
  </body>
</html>
```
