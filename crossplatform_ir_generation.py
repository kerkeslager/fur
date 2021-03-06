import collections

import conversion

def flatten(xses):
    return tuple(x for xs in xses for x in xs)

CIRProgram = collections.namedtuple(
    'CIRProgram',
    (
        'entry_list',
    ),
)

CIRLabel = collections.namedtuple(
    'CIRLabel',
    (
        'label',
    ),
)

CIRInstruction = collections.namedtuple(
    'CIRInstruction',
    (
        'instruction',
        'argument',
    ),
)

def generate_integer_literal(integer):
    return integer

def generate_string_literal(string):
    return '"{}"'.format(string)

def generate_symbol_literal(symbol):
    return 'sym({})'.format(symbol)

def generate_instruction_name_from_builtin(builtin):
    try:
        return {
            # Environment operations
            '__get__': 'get',

            # Integer operations
            '__add__': 'add',
            '__integer_divide__': 'idiv',
            '__modular_divide__': 'mod',
            '__multiply__': 'mul',
            '__negate__': 'neg',
            '__subtract__': 'sub',

            # Boolean operations
            '__eq__': 'eq',
            '__neq__': 'neq',
            '__lt__': 'lt',
            '__lte__': 'lte',
            '__gt__': 'gt',
            '__gte__': 'gte',

            # String operations
            '__concat__': 'concat',

            # Structure operations
            '__field__': 'field',
        }[builtin]

    except KeyError:
        import ipdb; ipdb.set_trace()

def generate_function_call_expression(counters, expression):
    if isinstance(expression.function_expression, conversion.CPSBuiltinExpression):
        return (
            (),
            (
                CIRInstruction(
                    instruction=generate_instruction_name_from_builtin(
                        expression.function_expression.symbol,
                    ),
                    argument=expression.argument_count,
                ),
            )
        )

    referenced_entry_list, instruction_list = generate_expression(
        counters,
        expression.function_expression,
    )

    instruction_list += (
        CIRInstruction(
            instruction='call',
            argument=expression.argument_count,
        ),
    )

    return referenced_entry_list, instruction_list

def generate_integer_literal_expression(counters, expression):
    referenced_entry_list = ()
    instruction_list = (CIRInstruction(
        instruction='push_integer',
        argument=generate_integer_literal(expression.integer),
    ),)

    return referenced_entry_list, instruction_list

def escape_name(name):
    return name.replace('$','$$').replace('_','$')

def generate_lambda_expression(counters, expression):
    if expression.name is None:
        name = '__lambda__'
    else:
        name = escape_name(expression.name)

    name_counter = counters.get(name, 0)
    counters[expression.name] = name_counter + 1
    label = '{}${}'.format(name, name_counter)

    referenced_entry_list_list = []
    instruction_list_list = []

    for statement in expression.statement_list:
        referenced_entry_list, instruction_list = generate_statement(counters, statement)
        referenced_entry_list_list.append(referenced_entry_list)
        instruction_list_list.append(instruction_list)

    # Pop from the stack in reversed order, because arguments were pushed onto
    # the stack in order
    argument_bindings = tuple(
        CIRInstruction(instruction='pop', argument='sym({})'.format(arg))
        for arg in reversed(expression.argument_name_list)
    )

    lambda_body = flatten(instruction_list_list)
    assert lambda_body[-1].instruction == 'drop'
    lambda_body = argument_bindings + lambda_body[:-1] + (CIRInstruction(instruction='return', argument=None),)

    referenced_entry_list_list.append(
        (CIRLabel(label=label),) + lambda_body,
    )

    instruction_list = (
        CIRInstruction(instruction='close', argument=label),
    )

    return flatten(referenced_entry_list_list), instruction_list

def generate_list_construct_expression(counters, expression):
    referenced_entry_list = ()
    instruction_list = (CIRInstruction(
        instruction='list',
        argument=2,
    ),)
    return referenced_entry_list, instruction_list

def generate_string_literal_expression(counters, expression):
    referenced_entry_list = ()
    instruction_list = (CIRInstruction(
        instruction='push_string',
        argument=generate_string_literal(expression.string),
    ),)

    return referenced_entry_list, instruction_list

def generate_structure_literal_expression(counters, expression):
    referenced_entry_list = ()
    instruction_list = (CIRInstruction(
        instruction='structure',
        argument=expression.field_count,
    ),)

    return referenced_entry_list, instruction_list

def generate_symbol_expression(counters, expression):
    referenced_entry_list = ()
    instruction_list = (CIRInstruction(
        instruction='push',
        argument=generate_symbol_literal(expression.symbol),
    ),)

    return referenced_entry_list, instruction_list

def generate_symbol_literal_expression(counters, expression):
    referenced_entry_list = ()
    instruction_list = (CIRInstruction(
        instruction='push_symbol',
        argument=generate_symbol_literal(expression.symbol),
    ),)

    return referenced_entry_list, instruction_list

def generate_variable_expression(counters, expression):
    referenced_entry_list = ()
    instruction_list = (CIRInstruction(
        instruction='push',
        argument=generate_symbol_literal(expression.variable),
    ),)

    return referenced_entry_list, instruction_list

def generate_expression(counters, expression):
    return {
        conversion.CPSFunctionCallExpression: generate_function_call_expression,
        conversion.CPSIfElseExpression: generate_if_else_expression,
        conversion.CPSIntegerLiteralExpression: generate_integer_literal_expression,
        conversion.CPSLambdaExpression: generate_lambda_expression,
        conversion.CPSListConstructExpression: generate_list_construct_expression,
        conversion.CPSStringLiteralExpression: generate_string_literal_expression,
        conversion.CPSStructureLiteralExpression: generate_structure_literal_expression,
        conversion.CPSSymbolExpression: generate_symbol_expression,
        conversion.CPSSymbolLiteralExpression: generate_symbol_literal_expression,
        conversion.CPSVariableExpression: generate_variable_expression,
    }[type(expression)](counters, expression)

def generate_expression_statement(counters, statement):
    referenced_entry_list, instruction_list = generate_expression(
        counters,
        statement.expression,
    )

    instruction_list += (
        CIRInstruction(
            instruction='drop',
            argument=None,
        ),
    )

    return referenced_entry_list, instruction_list

def generate_if_else_expression(counters, statement):
    if_counter = counters['if']
    counters['if'] += 1

    referenced_entry_list_list = []

    condition_referenced_entry_list, condition_instruction_list = generate_expression(
        counters,
        statement.condition_expression,
    )

    if_instruction_list_list = []
    for if_statement in statement.if_statement_list:
        referenced_entry_list, instruction_list = generate_statement(counters, if_statement)
        referenced_entry_list_list.append(referenced_entry_list)
        if_instruction_list_list.append(instruction_list)

    if_instruction_list = flatten(if_instruction_list_list)
    assert if_instruction_list[-1].instruction == 'drop'
    if_instruction_list = if_instruction_list[:-1]

    else_instruction_list_list = []

    for else_statement in statement.else_statement_list:
        referenced_entry_list, instruction_list = generate_statement(counters, else_statement)
        referenced_entry_list_list.append(referenced_entry_list)
        else_instruction_list_list.append(instruction_list)

    else_instruction_list = flatten(else_instruction_list_list)
    assert else_instruction_list[-1].instruction == 'drop'
    else_instruction_list = else_instruction_list[:-1]

    if_label = '__if${}__'.format(if_counter)
    else_label = '__else${}__'.format(if_counter)
    endif_label = '__endif${}__'.format(if_counter)

    instruction_list = condition_instruction_list + (
        CIRInstruction(
            instruction='jump_if_false',
            argument=else_label,
        ),
        CIRInstruction(
            instruction='jump',
            argument=if_label,
        ),
        CIRLabel(label=if_label),
    ) + if_instruction_list + (
        CIRInstruction(
            instruction='jump',
            argument=endif_label,
        ),
        CIRLabel(label=else_label),
    ) + else_instruction_list + (
        CIRLabel(label=endif_label),
    )

    return (
        condition_referenced_entry_list + flatten(referenced_entry_list_list),
        instruction_list,
    )

def generate_assignment_statement(counters, statement):
    referenced_entry_list, instruction_list = generate_expression(
        counters,
        statement.expression,
    )

    instruction_list += (
        CIRInstruction(
            instruction='pop',
            argument=generate_symbol_literal(statement.target),
        ),
    )

    return referenced_entry_list, instruction_list

def generate_push_statement(counters, statement):
    return generate_expression(counters, statement.expression)

def generate_variable_initialization_statement(counters, statement):
    referenced_entry_list, instruction_list = generate_expression(
        counters,
        statement.expression,
    )

    instruction_list += (
        CIRInstruction(
            instruction='pop',
            argument=generate_symbol_literal(statement.variable),
        ),
    )

    return referenced_entry_list, instruction_list

def generate_statement(counters, statement):
    return {
        conversion.CPSAssignmentStatement: generate_assignment_statement,
        conversion.CPSExpressionStatement: generate_expression_statement,
        conversion.CPSPushStatement: generate_push_statement,
        conversion.CPSVariableInitializationStatement: generate_variable_initialization_statement,
    }[type(statement)](counters, statement)

def generate(converted):
    referenced_entry_list_list = []
    instruction_list_list = []
    counters = {
        'if': 0,
    }

    for statement in converted.statement_list:
        referenced_entry_list, instruction_list = generate_statement(counters, statement)
        referenced_entry_list_list.append(referenced_entry_list)
        instruction_list_list.append(instruction_list)

    return CIRProgram(
        entry_list=flatten(referenced_entry_list_list) + (
            CIRLabel(label='__main__'),
        ) + flatten(instruction_list_list) + (
            CIRInstruction(instruction='end', argument=None),
        )
    )

NO_ARGUMENT_INSTRUCTIONS = set([
    'drop',
    'return',
])

def format_argument(arg):
    if arg is None:
        return 'nil'
    return arg

def output(program):
    lines = []

    for entry in program.entry_list:
        if isinstance(entry, CIRInstruction):
            if entry.instruction in NO_ARGUMENT_INSTRUCTIONS and entry.argument is None:
                lines.append('    {}'.format(entry.instruction))
            else:
                lines.append('    {} {}'.format(entry.instruction, format_argument(entry.argument)))

        if isinstance(entry, CIRLabel):
            lines.append('\n{}:'.format(entry.label))

    return '\n'.join(lines).lstrip()
