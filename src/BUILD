c_library(
    name = "adt",
    srcs = glob(["adt/*.c"]),
    hdrs = glob(["adt/*.h"]),
    deps = [":types"],
)

c_test(
    name = "adt_test",
    srcs = glob(["adt/tests/*.c"]),
    hdrs = glob(["adt/tests/*.h"]),
    deps = [
        ":adt",
        ":testing",
    ],
)

c_library(
    name = "testing",
    srcs = glob(["testing/*.c"]),
    hdrs = glob(["testing/*.h"]),
    deps = [":types"],
)

c_library(
    name = "types",
    srcs = glob(["types/*.c"]),
    hdrs = glob(["types/*.h"]),
)
