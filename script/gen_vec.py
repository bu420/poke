import itertools

def vec_operators(size):
    code = 'vec{0}<T> operator{1}(const vec{0}<T>& v) const {{return vec{0}<T>('
    if size >= 2:
        code += 'x(){1}v.x()'
        code += ',y(){1}v.y()'
    if size >= 3:
        code += ',z(){1}v.z()'
    if size == 4:
        code += ',w(){1}v.w()'
    code += ');}}\n'

    code += 'vec{0}<T> operator{1}(T scalar) const {{return vec{0}<T>('
    if size >= 2:
        code += 'x(){1}scalar'
        code += ',y(){1}scalar'
    if size >= 3:
        code += ',z(){1}scalar'
    if size == 4:
        code += ',w(){1}scalar'
    code += ');}}\n'

    code += 'vec{0}<T>& operator{1}=(const vec{0}<T>& v){{'
    if size >= 2:
        code += 'x(){1}=v.x();'
        code += 'y(){1}=v.y();'
    if size >= 3:
        code += 'z(){1}=v.z();'
    if size == 4:
        code += 'w(){1}=v.w();'
    code += 'return *this;}}\n'

    code += 'vec{0}<T>& operator{1}=(T scalar){{'
    if size >= 2:
        code += 'x(){1}=scalar;'
        code += 'y(){1}=scalar;'
    if size >= 3:
        code += 'z(){1}=scalar;'
    if size == 4:
        code += 'w(){1}=scalar;'
    code += 'return *this;}}\n'
    
    return code.format(size, '+') + code.format(size, '-') + code.format(size, '*') + code.format(size, '/')

def vec_simple_method(name, i):
    code = 'T& ' + name + '(){return arr[' + str(i) + '];}\n'
    code += 'const T& ' + name + '() const {return arr[' + str(i) + '];}\n'
    return code

def is_swizzle(method_name, vec_size):
    not_swizzle = {}
    not_swizzle[2] = ['xy', 'rg', 'uv']
    not_swizzle[3] = ['xyz', 'rgb', 'uvw']
    not_swizzle[4] = ['xyzw', 'rgba']

    if len(method_name) != vec_size:
        return True
    
    if (not_swizzle[vec_size].count(method_name) > 0):
        return False
    else:
        return True

def vec_swizzle_method(name_list, vec_size):
    return_type = 'vec{}<T>'.format(len(name_list))
    method_name = ''.join(name_list)

    code = return_type + ' ' + method_name + '() const {'
    if is_swizzle(method_name, vec_size):
        code += 'return ' + return_type + '('
        for name in name_list:
            code += name + '(),'
        code = code[:-1]
        code += ');'
    else:
        code += 'return *this;'
    code += '}\n'
    
    return code 

def vec_access_methods(size):
    code = ''

    if size >= 2:
        code += vec_simple_method('x', 0)
        code += vec_simple_method('y', 1)
        code += vec_simple_method('r', 0)
        code += vec_simple_method('g', 1)
        if size < 4:
            code += vec_simple_method('u', 0)
            code += vec_simple_method('v', 1)

    if size >= 3:
        code += vec_simple_method('z', 2)
        code += vec_simple_method('b', 2)
        if size < 4:
            code += vec_simple_method('w', 2)

    if size >= 4:
        code += vec_simple_method('w', 3)
        code += vec_simple_method('a', 3)

    if size >= 2:
        for i in list(itertools.permutations(['x', 'y'])):
            code += vec_swizzle_method(i, size)
        for i in list(itertools.permutations(['r', 'g'])):
            code += vec_swizzle_method(i, size)
        if size < 4:
            for i in list(itertools.permutations(['u', 'v'])):
                code += vec_swizzle_method(i, size)
        
    if size >= 3:
        for i in list(itertools.permutations(['x', 'y', 'z'])):
            code += vec_swizzle_method(i, size)
        for i in list(itertools.permutations(['r', 'g', 'b'])):
            code += vec_swizzle_method(i, size)
        if size < 4:
            for i in list(itertools.permutations(['u', 'v', 'w'])):
                code += vec_swizzle_method(i, size)

    if size >= 4:
        for i in list(itertools.permutations(['x', 'y', 'z', 'w'])):
            code += vec_swizzle_method(i, size)
        for i in list(itertools.permutations(['r', 'g', 'b', 'a'])):
            code += vec_swizzle_method(i, size)

    return code

def gen():
    code = '// vec2\n\n'
    code += vec_operators(2) + '\n'
    code += vec_access_methods(2) + '\n'
    code += '// vec3\n\n'
    code += vec_operators(3) + '\n'
    code += vec_access_methods(3) + '\n'
    code += '// vec4\n\n'
    code += vec_operators(4) + '\n'
    code += vec_access_methods(4) + '\n'
    return code

with open('vec.hpp', 'w') as f:
    print(gen(), file=f)