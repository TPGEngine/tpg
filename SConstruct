AddOption('--dbg', action='append_const', dest='cflags', const='-g')
AddOption('--opt', action='append_const', dest='cflags', const='-O3 -fno-math-errno -DNDEBUG')

import os
common_env=Environment(ENV=os.environ)
ccanada = ARGUMENTS.get('ccanada', 0)
common_env.Replace(CXX='mpic++')

if int(ccanada):
    common_env.Append(CPPDEFINES=['CCANADA'])

common_env.Append(CCFLAGS = ['-std=c++23', '-Wno-deprecated', '-Wall',
                                 '-Werror', '-Wextra', '-Wno-unused-parameter'])

common_env.MergeFlags(GetOption('cflags'))
common_env.Append(CPPDEFINES={'VERSION': 1})

# Our release build is derived from the common build environment...
release_env = common_env.Clone()
# ... and adds a RELEASE preprocessor symbol ...
release_env.Append(CPPDEFINES=['RELEASE'])
# ... and release builds end up in the "build/release" dir
release_env.VariantDir('build/release', 'src')

# # We define our debug build environment in a similar fashion...
# debug_env = common_env.Clone()
# debug_env.Append(CPPDEFINES=['DEBUG'])
# debug_env.VariantDir('build/debug', 'src')

# # Now that all build environment have been defined, let's iterate over
# # them and invoke the lower level SConscript files.
# for mode, env in dict(release=release_env, 
#     	       	      debug=debug_env).items():
#     env.SConscript('build/%s/SConscript' % mode, {'env': env})
for mode, env in dict(release=release_env).items():
    env.SConscript('build/%s/SConscript' % mode, {'env': env})    


