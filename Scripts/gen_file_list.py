# generate files.cmake that contains 
# set(header_files ... ) of every file in RHI/include
# set(<folder>_source_files ... ) of every file in RHI/source/<folder>

import os

def is_cxx_file(filename):
    return filename.endswith('.cpp') or filename.endswith('.hpp') or filename.endswith('.c') or filename.endswith('.h') 

def generate_file_list(dir):
    files: list[str] = []
    for (dirpath, _, filenames) in os.walk(dir):
        for filename in filenames:
            if is_cxx_file(filename):
                files.append(os.path.join(dirpath, filename))
    print('Adding files')
    for file in files: 
        print('\t - ' + file)
    return files

def save_to_cmake_file(files, variable_name, cmake_filename): 
    with open(cmake_filename, 'a+') as cmake_file:
        
        cmake_file.write('set('+ variable_name.upper() + '_FILES' +'\n')
        
        for file in files:
            cmake_file.write('\t"' + file.replace('\\', '/') + '"\n')
        cmake_file.write(')\n\n')

# path to the cmake file        
CMAKE_FILE_PATH = './FILES.cmake'

# remove old _FILES.cmake if exists
if os.path.exists(CMAKE_FILE_PATH):
    os.remove(CMAKE_FILE_PATH)
    
# generate _FILES.cmake
save_to_cmake_file(generate_file_list('Include'), 'HEADER', CMAKE_FILE_PATH)
save_to_cmake_file(generate_file_list('Source'), 'SOURCE', CMAKE_FILE_PATH)
