# generate files.cmake that contains 
# set(header_files ... ) of every file in RHI/include
# set(<folder>_source_files ... ) of every file in RHI/source/<folder>

import os

def is_cxx_file(filename):
    return filename.endswith('.cpp') or filename.endswith('.c')

def is_hxx_file(filename): 
    return filename.endswith('.hpp') or filename.endswith('.h')

def generate_file_list(dir):
    headers: list[str] = []
    sources: list[str] = []
    for (dirpath, _, filenames) in os.walk(dir):
        for filename in filenames:
            if is_cxx_file(filename):
                sources.append(os.path.join(dirpath, filename))
            elif is_hxx_file(filename):
                headers.append(os.path.join(dirpath, filename))
                
    print('Adding source files')
    for source_file in sources: 
        print('\t - ' + source_file)
    return (headers, sources)

def save_to_cmake_file(files, variable_name, cmake_filename): 
    with open(cmake_filename, 'a+') as cmake_file:
        
        cmake_file.write('set('+ variable_name.upper() + '_FILES' +'\n')
        
        for file in files:
            cmake_file.write('\t"' + file.replace('\\', '/') + '"\n')
        cmake_file.write(')\n\n')

# path to the cmake file        
CMAKE_FILE_PATH = './cmake/source-files.cmake'

# remove old _FILES.cmake if exists
if os.path.exists(CMAKE_FILE_PATH):
    os.remove(CMAKE_FILE_PATH)
    
# generate _FILES.cmake
# include dir
(headers, sources) = generate_file_list('Include')
HEADERS = headers
SOURCES = sources


(headers, sources) = generate_file_list('Source')
HEADERS = HEADERS + headers
SOURCES = SOURCES + sources


save_to_cmake_file(HEADERS, 'HEADER', CMAKE_FILE_PATH)
save_to_cmake_file(SOURCES, 'SOURCE', CMAKE_FILE_PATH)