# Copyright (C) 2010-2017 Apple Inc. All rights reserved.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions
# are met:
# 1.  Redistributions of source code must retain the above copyright
#     notice, this list of conditions and the following disclaimer.
# 2.  Redistributions in binary form must reproduce the above copyright
#     notice, this list of conditions and the following disclaimer in the
#     documentation and/or other materials provided with the distribution.
#
# THIS SOFTWARE IS PROVIDED BY APPLE INC. AND ITS CONTRIBUTORS ``AS IS'' AND
# ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
# WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
# DISCLAIMED. IN NO EVENT SHALL APPLE INC. OR ITS CONTRIBUTORS BE LIABLE FOR
# ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
# DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
# SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
# CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
# OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
# OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

import re
import sys

from generator import model

g_inner_type = ['pcfetcher_string']

def combine_condition(conditions):
    if conditions:
        if len(conditions) == 1:
            return conditions[0]
        else:
            return bracket_if_needed(' && '.join(map(bracket_if_needed, conditions)))
    else:
        return None


def bracket_if_needed(condition):
    if re.match(r'.*(&&|\|\|).*', condition):
        return '(%s)' % condition
    else:
        return condition

def parse(file):
    receiver_attributes = None
    destination = None
    name = None
    messages = []
    conditions = []
    master_condition = None
    superclass = []
    for line in file:
        line = line.strip()
        match = re.search(r'msg -> (?P<name>[A-Za-z_0-9]+) \s*(?::\s*(?P<superclass>.*?) \s*)?(?:(?P<attributes>.*?)\s+)?{', line)
        if match:
            receiver_attributes = parse_attributes_string(match.group('attributes'))
            if match.group('superclass'):
                superclass = match.group('superclass')
            if conditions:
                master_condition = conditions
                conditions = []
            name = 'msg_' + match.group('name')
            continue
        match = re.search(r'(.*);', line)
        if match:
            parameters_string = match.groups()
            if parameters_string:
                parameters = parse_parameters_string(parameters_string)
                for parameter in parameters:
                    parameter.condition = combine_condition(conditions)
            else:
                parameters = []

            messages.append(model.Message(name, parameters, None, None, combine_condition(conditions)))
    return model.MessageReceiver(name, superclass, receiver_attributes, messages, combine_condition(master_condition))


def parse_attributes_string(attributes_string):
    if not attributes_string:
        return None
    return attributes_string.split()


def split_parameters_string(parameters_string):
    parameters = []
    current_parameter_string = ''

    nest_level = 0
    for character in parameters_string:
        if character == ',' and nest_level == 0:
            parameters.append(current_parameter_string)
            current_parameter_string = ''
            continue

        if character == '<':
            nest_level += 1
        elif character == '>':
            nest_level -= 1

        current_parameter_string += character

    parameters.append(current_parameter_string)
    return parameters


def parse_parameters_string(parameters_string):
    parameters = []

    for parameter_string in split_parameters_string(parameters_string):
        match = re.search(r'\s*(?:\[(?P<attributes>.*?)\]\s+)?(?P<type_and_name>.*)', parameter_string)
        attributes_string, type_and_name_string = match.group('attributes', 'type_and_name')

        split = type_and_name_string.rsplit(' ', 1)
        parameter_kind = 'base'
        if split[0].startswith('array '):
            parameter_kind = 'array'
            split[0] = split[0][6:]
        elif split[0].startswith('struct '):
            parameter_kind = 'struct'
            split[0] = split[0][7:]
        elif split[0].startswith('enum:'):
            parameter_kind = split[0][:split[0].find(' ')]
            split[0] = split[0][split[0].find(' ') + 1:]

        parameter_type = split[0]
        parameter_name = split[1]

        parameters.append(model.Parameter(kind=parameter_kind, type=parameter_type, name=parameter_name, attributes=parse_attributes_string(attributes_string)))
    return parameters

def gen_msg_header(receiver):
    result = []

    result.append('#include "pcfetcher_msg.h"\n')
    result.append('\n')

    for parameter in receiver.iterparameters():
        kind = parameter.kind
        type = parameter.type
        if type in g_inner_type:
            continue

        if kind == 'struct':
            result.append('#include "%s.h"\n' % type)

    result.append('\n')
    result.append('struct pcfetcher_%s {\n' % receiver.name)
    result.append('    struct pcfetcher_msg_header header;\n')
    result.append('\n')

    for parameter in receiver.iterparameters():
        kind = parameter.kind
        type = parameter.type
        name = parameter.name
        if kind == 'base':
            result.append('    %s %s;\n' % (type, name))
        elif kind == 'struct':
            result.append('    struct %s* %s;\n' % (type, name))
        elif kind == 'array':
            result.append('\n')
            result.append('    // %s array\n' % type)
            result.append('    array_list_free_fn* %s_free_fn;\n' % name)
            result.append('    struct pcutils_arrlist* %s;\n' % name)
            result.append('\n')

    result.append('}\n\n')

    result.append('struct pcfetcher_%s* pcfetcher_%s_create(void);\n\n' % (receiver.name, receiver.name))
    result.append('void pcfetcher_%s_destroy(struct pcfetcher_msg_%s*);\n\n' % (receiver.name, receiver.name))
    result.append('void pcfetcher_%s_encode(pcfetcher_encoder*, void*);\n\n' % receiver.name)
    result.append('bool pcfetcher_%s_decode(pcfetcher_decoder*, void**);\n\n' % receiver.name)

    result.append('\n')
    result.append('static inline void pcfetcher_%s_array_free_fn(void* v)\n' % receiver.name)
    result.append('{\n')
    result.append('    pcfetcher_%s_destroy((struct pcfetcher_%s*)v);\n' % (receiver.name, receiver.name))
    result.append('}\n')

    result.append('\n')
    result.append('static inline struct pcutils_arrlist* pcfetcher_%s_array_create(void)\n' % receiver.name)
    result.append('{\n')
    result.append('    return pcfetcher_array_create(pcfetcher_%s_array_free_fn);\n' % receiver.name)
    result.append('}\n')

    result.append('\n')
    result.append('static inline void pcfetcher_%s_array_destory(struct pcutils_arrlist* array)\n')
    result.append('{\n')
    result.append('    pcfetcher_array_destroy(array);\n')
    result.append('}\n')

    result.append('\n')
    result.append('static inline void pcfetcher_%s_array_encode(\n' % receiver.name)
    result.append('        struct pcfetcher_encoder* encoder, struct pcutils_arrlist* array)\n')
    result.append('{\n')
    result.append('    pcfetcher_array_encode(encoder, array, pcfetcher_%s_encode);\n' % receiver.name)
    result.append('}\n')

    result.append('\n')
    result.append('static inline void pcfetcher_%s_array_decode(\n' % receiver.name)
    result.append('        struct pcfetcher_decoder* decoder, struct pcutils_arrlist* array)\n')
    result.append('{\n')
    result.append('    pcfetcher_array_decode(decoder, array, pcfetcher_%s_decode);\n' % receiver.name)
    result.append('}\n')

    return ''.join(result)

def gen_msg_source(receiver):
    result = []
    msg_name = receiver.name

    result.append('#include "pcfetcher_%s.h"\n' % msg_name)
    result.append('\n')

    result.append('struct pcfetcher_%s* pcfetcher_%s_create(void)\n' % (msg_name, msg_name))
    result.append('{\n')
    result.append('    return (struct pcfetcher_%s*)\n' % (msg_name));
    result.append('            calloc(sizeof(struct pcfetcher_%s), 1);\n' % msg_name);
    result.append('}\n\n')

    result.append('void pcfetcher_%s_destroy(struct pcfetcher_msg_%s* msg)\n' % (msg_name, msg_name))
    result.append('{\n')
    result.append('    if (!msg) {\n');
    result.append('        return;\n');
    result.append('    }\n');
    for parameter in receiver.iterparameters():
        kind = parameter.kind
        type = parameter.type
        name = parameter.name
        if kind == 'struct':
            result.append('    %s_destroy(msg->%s);\n' % (type, name))
        elif kind == 'array':
            result.append('    %s_array_destroy(msg->%s);\n' % (type, name))
    result.append('}\n\n')

    result.append('void pcfetcher_%s_encode(pcfetcher_encoder* encode, void* v)\n' % msg_name)
    result.append('{\n')
    result.append('    struct pcfetcher_%s* msg = (struct pcfetcher_%s*)v;\n' % (msg_name, msg_name))

    for parameter in receiver.iterparameters():
        kind = parameter.kind
        type = parameter.type
        name = parameter.name
        if kind == 'base':
            result.append('    pcfetcher_base_encode(encoder, msg->%s);\n' % name)
        elif kind == 'struct':
            result.append('    %s_encode(encoder, msg->%s);\n' % (type, name))
        elif kind == 'array':
            result.append('    %s_array_encode(encoder, msg->%s);\n' % (type, name))

    result.append('}\n\n')

    result.append('bool pcfetcher_%s_decode(pcfetcher_decoder* decoder, void** v)\n' % msg_name)
    result.append('{\n')
    result.append('    struct pcfetcher_%s* msg = pcfetcher_%s_create()\n' % (msg_name, msg_name));

    for parameter in receiver.iterparameters():
        kind = parameter.kind
        type = parameter.type
        name = parameter.name
        if kind == 'base':
            result.append('    pcfetcher_base_decode(decoder, msg->%s);\n' % name)
        elif kind == 'struct':
            result.append('    %s_decode(decoder, &msg->%s);\n' % (type, name))
        elif kind == 'array':
            result.append('    %s_array_decode(decoder, msg->%s);\n' % (type, name))

    result.append('}\n\n')

    return ''.join(result)

def main(argv):
    receiver = None
    with open(argv[1]) as source_file:
        receiver = parse(source_file)

    receiver_name = receiver.name
    with open('pcfetcher_%s.h' % receiver_name, "w+") as header_output:
        header_output.write(gen_msg_header(receiver))

    with open('pcfetcher_%s.c' % receiver_name, "w+") as implementation_output:
        implementation_output.write(gen_msg_source(receiver))
    return 0

if __name__ == '__main__':
    sys.exit(main(sys.argv))
