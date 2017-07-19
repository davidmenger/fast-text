{
    "targets": [
        {
            "target_name": "fasttext",
            "sources": [
                "lib/src/args.cc",
                "lib/src/args.h",
                "lib/src/dictionary.cc",
                "lib/src/dictionary.h",
                "lib/src/fasttext.cc",
                "lib/src/fasttext.h",
                "lib/src/matrix.cc",
                "lib/src/matrix.h",
                "lib/src/model.cc",
                "lib/src/model.h",
                "lib/src/productquantizer.cc",
                "lib/src/productquantizer.h",
                "lib/src/qmatrix.cc",
                "lib/src/qmatrix.h",
                "lib/src/real.h",
                "lib/src/utils.cc",
                "lib/src/utils.h",
                "lib/src/vector.cc",
                "lib/src/vector.h",
                "src/classifier.h",
                "src/classifierWorker.cc",
                "src/query.h",
                "src/nnWorker.cc",
                "src/wrapper.cc",
                "src/fasttext.cc"
            ],
            "include_dirs": ["<!(node -e \"require('nan')\")"],
            "cflags": [
                "-std=c++11",
                "-pthread",
                "-Wsign-compare",
                "-fexceptions",
                "-O0"
            ],
            "conditions": [
                [ 'OS!="win"', {
                    "cflags+": [ "-std=c++11", "-fexceptions" ],
                    "cflags_c+": [ "-std=c++11", "-fexceptions" ],
                    "cflags_cc+": [ "-std=c++11", "-fexceptions" ],
                }],
                [ 'OS=="mac"', {
                    "cflags+": [ "-stdlib=libc++" ],
                    "xcode_settings": {
                        "OTHER_CPLUSPLUSFLAGS" : [ "-std=c++11", "-stdlib=libc++", "-pthread" ],
                        "OTHER_LDFLAGS": [ "-stdlib=libc++" ],
                        "MACOSX_DEPLOYMENT_TARGET": "10.7",
                        "GCC_ENABLE_CPP_EXCEPTIONS": "YES",
                        "CLANG_CXX_LANGUAGE_STANDARD":"c++11",
                        "CLANG_CXX_LIBRARY": "libc++"
                    },
                }]
            ]
        }
    ]
}