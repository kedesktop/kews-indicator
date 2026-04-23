project('kews-indicator', 'cpp',
        meson_version:   '>=1.4.0',
        version:         '0.0.1',
        license:         'GPL-3.0-or-later',
        license_files:   [ 'COPYING' ],
        default_options: [ 'warning_level=3', 'cpp_std=c++23' ])

deps = [ dependency('gtkmm-4.0'),
         dependency('gtk4-layer-shell-0'),
         dependency('wireplumber-0.5'),
         dependency('jsoncpp', fallback: [ 'jsoncpp', 'jsoncpp_dep' ]), ]

subdir('src')
executable('kews-indicator', src,
           include_directories: include_directories('include'),
           dependencies:        deps)