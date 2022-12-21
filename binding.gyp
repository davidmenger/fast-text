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
                "src/nodeArgument.cc",
                "src/nodeArgument.h",
                "src/classifier.h",
                "src/classifierWorker.cc",
                "src/query.h",
                "src/trainWorker.cc",
                "src/trainWorker.h",
                "src/vectorWorker.cc",
                "src/vectorWorker.h",
                "src/nnWorker.cc",
                "src/nnWorker.h",
                "src/wrapper.cc",
                "src/wrapper.h",
                "src/fasttext.cc"
            ],
            "include_dirs": ["<!(node -e \"require('nan')\")"],
            "cflags": [
                "-std=c++14",
                "-pthread",
                "-fexceptions",
                "-O3",
                "-Wsign-compare",
                "-Wall",
                "-march=native",
                "-frtti"
            ],
            "conditions": [
                [ "OS=='linux'", {
                    "cflags+": [ "-std=c++14", "-fexceptions", "-Wall", "-Wsign-compare", "-frtti" ],
                    "cflags_c+": [ "-std=c++14", "-fexceptions", "-Wall", "-Wsign-compare", "-frtti"],
                    "cflags_cc+": [ "-std=c++14", "-fexceptions", "-Wall", "-Wsign-compare", "-frtti" ],
                }],
                [ "OS=='freebsd'", {
                    "cflags+": [ "-std=c++17", "-fexceptions", "-Wall", "-Wsign-compare", "-frtti" ],
                    "cflags_c+": [ "-std=c++17", "-fexceptions", "-Wall", "-Wsign-compare", "-frtti" ],
                    "cflags_cc+": [ "-std=c++17", "-fexceptions", "-Wall", "-Wsign-compare", "-frtti" ],
                }],
                [ 'OS=="mac"', {
                    "cflags+": [ "-stdlib=libc++", "-Wsign-compare", "-frtti" ],
                    "xcode_settings": {
                        "OTHER_CPLUSPLUSFLAGS" : [ "-std=c++14", "-stdlib=libc++", "-pthread", "-Wsign-compare", "-frtti" ],
                        "OTHER_LDFLAGS": [ "-stdlib=libc++", "-frtti" ],
                        "MACOSX_DEPLOYMENT_TARGET": "10.11",
                        "GCC_ENABLE_CPP_EXCEPTIONS": "YES",
                        "CLANG_CXX_LIBRARY": "libc++"
                    },
                }],
                [
                    "OS=='win'", {
                        "cflags": [
                        "-Wall"
                        ],
                        "defines": [
                        "WIN"
                        ],
                        "msvs_settings": {
                        "VCCLCompilerTool": {
                            "ExceptionHandling": "2",
                            "DisableSpecificWarnings": [
                            "4244"
                            ],
                        },
                        "VCLinkerTool": {
                            "LinkTimeCodeGeneration": 1,
                            "OptimizeReferences": 2,
                            "EnableCOMDATFolding": 2,
                            "LinkIncremental": 1,
                        }
                    }
                }]
            ]
        }
    ]
}