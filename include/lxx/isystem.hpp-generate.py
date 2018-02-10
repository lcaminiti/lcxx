
# Copyright (C) 2014 Lorenzo Caminiti
# Distributed under the Boost Software License, Version 1.0
# (see accompanying file LICENSE_1_0.txt or a copy at
# http://www.boost.org/LICENSE_1_0.txt)
# Home at http://sourceforge.net/projects/contractpp

max_includes = 50

import os
import sys
import shutil

includes = ''
none_defined = ''
and_ = '       '
for i in range(0, max_includes):
    includes = includes + str('''
#ifdef LXX_ISYSTEM{0}
    #include LXX_ISYSTEM{0}
#endif
#undef LXX_ISYSTEM{0}
    ''').format(i)
    none_defined = none_defined + and_ + str(
            '!defined(LXX_ISYSTEM{0})').format(i)
    and_ = ' \\\n        && '
        
print('''
// WARNING: FILE AUTOMATICALLY GENERATED, DO NOT MODIFY IT!
// Instead, modify related generation script and run:
//   python "{0}" > isystem.hpp

// No #include guard here.

#if {1}
    #error "Included \\"isystem.hpp\\" with no `LXX_ISYSTEM<n>` header defined"
#endif

// Compiler-specific disable all warnings but only for following #includes.
#ifdef _MSVC_VER
    #pragma warning(push, 0)
#endif
{2}
// Compiler-specific re-enabled warnings at level before disable above.
#ifdef _MSVC_VER
    #pragma warning(pop)
#endif
'''.format(os.path.basename(sys.argv[0]), none_defined, includes))

