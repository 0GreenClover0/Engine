import os
import regex as re
import keyboard
import argparse

menu = []
active_choice = 0
scene_serializer_lines = ""

def find_serializable_variables(header_file_path, all_public):
    
    in_public_section = False

    non_serializable = False

    if not os.path.isfile(header_file_path):
        file_name = os.path.basename(header_file_path)
        for root, dirs, files in os.walk(args.engine_dir + '/src'):
            if file_name in files:
                header_file_path = os.path.join(root, file_name)
                break

        if not os.path.isfile(header_file_path):
            print('File ' + header_file_path + ' does not exist')
            return []

    serializable_vars = []

    variable_pattern = re.compile(r'^\s*([\w\:\<\>]+)\s+(\w+)\s*=\s*(([a-zA-Z0-9 \.\,\{\}\"\:\_])*([\"].*[\"])*(\(([a-zA-Z0-9 \.\,\{\}\"\:\_]*([\"].*[\"])*|(?6))*\))*)+\s*;')

    with open(header_file_path, 'r') as file:
        lines = file.readlines()

        for line in lines:
            
            if not in_public_section:
                if 'public:' in line:
                    in_public_section = True
                continue

            if non_serializable == True and all_public == False:
                non_serializable = False;
                continue

            if 'NON_SERIALIZED' in line and all_public == False:
                non_serializable = True
                continue

            if 'private:' in line or 'protected:' in line:
                return serializable_vars

            match = variable_pattern.match(line)
            if match:
                var_type = match.group(1)
                var_name = match.group(2)
                serializable_vars.append((var_type, var_name, True))

    return serializable_vars

def find_inherits_from(header_file_path):
    if not os.path.isfile(header_file_path):
        file_name = os.path.basename(header_file_path)
        for root, dirs, files in os.walk(args.engine_dir + '/src'):
            if file_name in files:
                header_file_path = os.path.join(root, file_name)
                break

    with open(header_file_path, 'r') as file:
        for line_number, line in enumerate(file):
            if ': public ' in line:
                # Find the position of ': public ' in the line
                index = line.index(': public ') + len(': public ')
                # Get the substring starting from the position after ': public '
                inherits_from = line[index:].split()[0]
                return inherits_from

# NOTE: This doesn't include the serializable variables from the header_file_path!
def recursively_search_serializable_variables(header_file_path):
    inherits_from = find_inherits_from(header_file_path)

    if inherits_from == "Component":
        return []

    inherits_from_path = args.engine_dir + '/src/' + inherits_from + '.h'

    if not os.path.isfile(inherits_from_path):
        file_name = os.path.basename(inherits_from_path)
        for root, dirs, files in os.walk(args.engine_dir + '/src'):
            if file_name in files:
                inherits_from_path = os.path.join(root, file_name)
                break

    return find_serializable_variables(inherits_from_path, False) + recursively_search_serializable_variables(inherits_from_path)

def display_menu(menu_items, active_index):
    os.system('cls')
    print('Choose variable to include in serialization')
    print('up, down, space, enter')
    option = ""
    for i, item in enumerate(menu_items):
        var_type, var_name, is_checked = item
        if i == active_index:
            option ='> ' + var_type + ' ' + var_name
        else:
            option ='  ' + var_type + ' ' + var_name

        if is_checked == True:
            option += ' [x]'
        else:
            option += ' [ ]'

        print(option)

def check_includes(Component, file = '/src/SceneSerializer.cpp'):
    global scene_serializer_lines

    print(Component)

    header_pattern = re.compile('#include "' + Component.replace(args.engine_dir + "/src/", "", 1) + '"')
    file_path = args.engine_dir + file

    if file == '/src/SceneSerializer.cpp':
        lines = scene_serializer_lines
    else:
        with open(file_path, 'r') as file_content:
            lines = file_content.readlines()

    for line in lines:
        if header_pattern.match(line):
                    return True

    return False

def remove_lines_between(start_line, end_line, first_skip = False, file = '/src/SceneSerializer.cpp'):
    global scene_serializer_lines
    print('Removing lines from ' + start_line + ' to ' + end_line)

    file_path = args.engine_dir + file
    lines = None
    from_cache = False

    if file == '/src/SceneSerializer.cpp':
        lines = scene_serializer_lines
        from_cache = True
    else:
        with open(file_path, 'r') as file:
            lines = file.readlines()

    save_lines = True

    new_lines = []
    
    for line in lines:
        if start_line in line:
            save_lines = False

        if save_lines:
            new_lines.append(line)
        else:
            if end_line in line:
                if first_skip == True:
                    first_skip = False
                else:
                    save_lines = True

    if from_cache:
        scene_serializer_lines.clear()
        scene_serializer_lines += new_lines
    else:
        with open(file_path, 'w') as file:
            file.writelines(new_lines)

def add_lines_at_target(target_line, lines_to_add, shift = 0, file = '/src/SceneSerializer.cpp'):
    global scene_serializer_lines

    file_path = args.engine_dir + file
    lines = None
    from_cache = False

    if file == '/src/SceneSerializer.cpp':
        lines = scene_serializer_lines
        from_cache = True
    else:
        with open(file_path, 'r') as file:
            lines = file.readlines()

    target_index = None
    for index, line in enumerate(lines):
        if target_line in line:
            target_index = index + shift
            break

    if target_index is None:
        print("Can't find place to add code for new componenet serialization")
        return

    for line_to_add in reversed(lines_to_add):
        lines.insert(target_index, line_to_add + '\n')

    if from_cache:
        scene_serializer_lines = lines
    else:
        with open(file_path, 'w') as file:
            file.writelines(lines)

def create_header_code(Component):
    header_code = [
        '#include "' + Component.replace(args.engine_dir + "/src/", "", 1) +'"',
    ]

    return header_code

def create_serialization_code(file, Component, serializable_vars, indentation = 0):

    name, parent, is_parent, is_abstract = file

    component = Component.lower()

    serialization_code = [
        '    if (auto const ' + component + ' = std::dynamic_pointer_cast<class ' + Component + '>(component); ' + component + ' != nullptr)',
        '    {'
    ]

    if parent == 'Component':
        serialization_code += [
        '        out << YAML::BeginMap;'
        ]

    if is_parent == True:
        serialization_code += [
        '        // # Put new ' + Component  + ' kid here',
        '        {',
        '            out << YAML::Key << "ComponentName" << YAML::Value << "' + Component + 'Component";',
        '            out << YAML::Key << "guid" << YAML::Value << ' + component + '->guid;'
        ]
    else:
        serialization_code += [
        '        out << YAML::Key << "ComponentName" << YAML::Value << "' + Component + 'Component";',
        '        out << YAML::Key << "guid" << YAML::Value << ' + component + '->guid;'
        ]

    if is_parent == True:
        serialization_code += [
        '        }'
        ]

    for var_type, var_name, is_checked in serializable_vars:

        if is_checked == False:
            continue

        serialization_code += [
        '        out << YAML::Key << "' + var_name + '" << YAML::Value << ' + component + '->' + var_name + ';',
    ]    

    if parent == 'Component':
        serialization_code += [
        '        out << YAML::EndMap;',
        ]

    serialization_code += [
        '    }',
    ]

    #if parent != 'Component':
    serialization_code += [
    '    else'
    ]

    for i in range(len(serialization_code)):
        for j in range(indentation):
            serialization_code[i] = '    ' + serialization_code[i]

    return serialization_code

def create_deserialization_code(Component, serializable_vars):

    deserialization_code = [
        '    if (component_name == "' + Component + 'Component")',
        '    {',
        '        if (first_pass)',
        '        {',
        '            auto const deserialized_component = ' + Component + '::create();',
        '            deserialized_component->guid = component["guid"].as<std::string>();',
        '            deserialized_pool.emplace_back(deserialized_component);',
        '        }',
        '        else',
        '        {',
        '            auto const deserialized_component = std::dynamic_pointer_cast<class ' + Component + '>(get_from_pool(component["guid"].as<std::string>()));'
    ]

    for var_type, var_name, is_checked in serializable_vars:
        
        if is_checked == False:
            continue

        deserialization_code += [
            '            deserialized_component->' + var_name + ' = component["' + var_name + '"].as<' + var_type + '>();',
        ]

    deserialization_code += [
        '            deserialized_entity->add_component(deserialized_component);',
        '            deserialized_component->reprepare();',
        '        }',
        '    }',
        '        else'
    ]

    return deserialization_code

def pick_variables(serializable_vars):
    
    menu = serializable_vars
    active_choice = 0

    display_menu(menu, active_choice)

    while True:
        event = keyboard.read_event()
        if event.event_type == keyboard.KEY_DOWN:
            key = event.name

            if key == 'up':
                active_choice = (active_choice - 1) % len(menu)
                display_menu(menu, active_choice)

            elif key == 'down':
                active_choice = (active_choice + 1) % len(menu)
                display_menu(menu, active_choice)

            elif key == 'space':
                option = menu[active_choice]
                var_type, var_name, is_checked = option
                menu[active_choice] = (var_type, var_name, not is_checked)
                display_menu(menu, active_choice)

            elif key == 'enter':
                os.system('cls')
                break

    return serializable_vars

def scan_all_files(inherits_from = 'Component'):

    header_folder_path = args.engine_dir + '/src'
    files_to_serialize = []

    for root, dirs, files in os.walk(header_folder_path):
        for file in files:
            potentially_non_serialized_class = False
            is_checked = False
            is_abstract = True
            if file.endswith('.h'):
                with open(os.path.join(root, file), 'r', encoding='utf-8') as f:
                    for line in f:
                        if potentially_non_serialized_class == True:
                            if 'class' in line:
                                potentially_non_serialized_class = False
                                # Class is not serialized, abort reading this file
                                break

                        if 'NON_SERIALIZED' in line:
                            potentially_non_serialized_class = True
                            continue

                        if ': public ' + inherits_from in line and is_checked == False:
                            files_to_serialize.append((os.path.join(root, file), inherits_from, False, is_abstract))
                            is_checked = True

                        if ' create()' in line and is_checked == True:
                            is_abstract = False
                            _name, _parent, _is_parent, _is_abstract = files_to_serialize[-1]
                            files_to_serialize[-1] = (_name, _parent, _is_parent, is_abstract)
                            break;

    
    for i in range(len(files_to_serialize)):
        file = files_to_serialize[i]
        name, parent, is_parent, is_abstract = file
        kids = scan_all_files(os.path.basename(name.replace("\\", "/"))[:-2])
        if kids != []:
            is_parent = True
            file = (name, parent, is_parent, is_abstract)
            files_to_serialize[i] = file
        files_to_serialize += kids

    return files_to_serialize

def clean_up(files_to_serilize, Component, is_parent):

    if args.pick_files:
        component = Component.lower()
        for file in files_to_serilize:
            new_name, new_parent, new_is_parent, new_is_abstract = file
            new_Component = os.path.basename(new_name)[:-2]
            if new_parent == Component:
                clean_up(files_to_serilize, new_Component, new_is_parent)
        
        if is_parent:
            remove_lines_between('if (auto const ' + component + ' = std::dynamic_pointer_cast<class ' + Component + '>(component); ' + component + ' != nullptr)', 'else', True)
        else:
            remove_lines_between('if (auto const ' + component + ' = std::dynamic_pointer_cast<class ' + Component + '>(component); ' + component + ' != nullptr)', 'else')
        remove_lines_between('if (component_name == "' + Component + 'Component")', 'else')

def add_serialization(file, pick_vars, pick_files, indentation = 0):

    name, parent, is_parent, is_abstract = file
    name = name.replace("\\", "/")

    if pick_files:
        yn = input('Serialize ' + name + '? y/n/e ')
        if yn == 'n':
            return
        elif yn == 'e':
            exit()

    Component = os.path.basename(name)[:-2]
    header_file_path = args.engine_dir + '/src/' + Component + '.h'
    serializable_vars = find_serializable_variables(header_file_path, pick_vars)
    
    print('Adding ' + Component)
    print(serializable_vars)

    if pick_vars == True:
        serializable_vars = pick_variables(serializable_vars)
    
    is_already_serialized = check_includes(name)

    create_serialization_code(file, Component, serializable_vars)

    if is_already_serialized == False:
        add_lines_at_target('// # Put new header here', create_header_code(name))
    else:
        clean_up(files_to_serialize, Component, is_parent)
        
    place_to_add_serialization = '// # Put new serialization here'
    shift = -5
    if parent != 'Component':
        place_to_add_serialization = '// # Put new ' + parent + ' kid here'
        shift = 1
        
    add_lines_at_target(place_to_add_serialization, create_serialization_code(file, Component, serializable_vars, indentation), shift)
    print('Succesful added serialization for ' + Component + '!')

    additional_variables = recursively_search_serializable_variables(header_file_path)

    print("Additional variables from parents added to deserialization code: ")
    print(additional_variables)

    if is_abstract == False:
        add_lines_at_target('// # Put new deserialization here', create_deserialization_code(Component, serializable_vars + additional_variables), -3)
        print('Succesful added deserialization for ' + Component + '!')

    components_to_remove = []

    for file in files_to_serialize:
        new_name, new_parent, new_is_parent, new_is_abstract = file
        if new_parent == Component:
            components_to_remove.append(new_name)
            add_serialization(file, pick_vars, pick_files, indentation + 1)

    for trash in components_to_remove:
        index = 0
        while index < len(files_to_serialize):
            if files_to_serialize[index][0] == trash:
                del files_to_serialize[index]
            else:
                index += 1

def add_to_component_list(file):
    name, parent, is_parent, is_abstract = file
    name = name.replace("\\", "/")
    Component = os.path.basename(name)[:-2]
    readable = re.sub(r'(?<=[a-z])(?=[A-Z])', ' ', Component)

    if is_abstract:
        return
    
    print('Adding ' + Component + ' to component list')
    add_lines_at_target('// # Put new component here', ['    ENUMERATE_COMPONENT(' + Component + ', "' + readable + '") \\'], 0, '/src/ComponentList.h')
    
    is_already_serialized = check_includes(name, '/src/Editor.cpp')

    if is_already_serialized == False:
        add_lines_at_target('// # Put new header here', create_header_code(name), 0, '/src/Editor.cpp')


parser = argparse.ArgumentParser(description='Engine Header Tool')
parser.add_argument('-d', '--engine_dir', action='store', help="root directory of the engine")
parser.add_argument('-pv', '--pick_vars', action='store_true', help='let you pick variables to serilize')
parser.add_argument('-pf', '--pick_files', action='store_true', help='let you pick files to serilize')

args = parser.parse_args()

with open(args.engine_dir + '/src/SceneSerializer.cpp', 'r') as file:
    scene_serializer_lines = file.readlines()

files_to_serialize = scan_all_files()
sorted_files = sorted(files_to_serialize, key=lambda x: x[1] != "Component")
for i in range(len(files_to_serialize)):
    print(files_to_serialize[i])

remove_lines_between('// # Auto serialization start', '// # Put new serialization here')
code = [
    '    // # Auto serialization start',
    '    {',
    '        // NOTE: This only returns unmangled name while using the MSVC compiler',
    '        std::string const name = typeid(*component).name();',
    '        std::cout << "Error. Serialization of component " << name.substr(6) << " failed." << "\\n";',
    '    }',
    '    // # Put new serialization here'
]
add_lines_at_target('auto_serialize_component', code, 2)

remove_lines_between('// # Auto deserialization start', '// # Put new deserialization here')
code = [
    '    // # Auto deserialization start',
    '    {',
    '        std::cout << "Error. Deserialization of component " << component_name << " failed." << "\\n";',
    '    }',
    '    // # Put new deserialization here'
]
add_lines_at_target('auto_deserialize_component', code, 4)

remove_lines_between('// # Auto component list start', '// # Auto component list end', False, '/src/ComponentList.h')
add_lines_at_target('// # Put new component here', ['    // # Auto component list start'], 0, '/src/ComponentList.h')
add_lines_at_target('// # Put new component here', ['#define ENUMERATE_COMPONENTS \\'], 0, '/src/ComponentList.h')

for file in files_to_serialize:
    add_to_component_list(file)
for file in files_to_serialize:
    add_serialization(file, args.pick_vars, args.pick_files)

add_lines_at_target('// # Put new component here', ['    // # Auto component list end'], 0, '/src/ComponentList.h')

with open(args.engine_dir + '/src/SceneSerializer.cpp', 'w') as file:
    file.truncate(0)
    file.writelines(scene_serializer_lines)

#cmd = input()
print("\033[A                             \033[A")
