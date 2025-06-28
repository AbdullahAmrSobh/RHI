#include "ShaderCompiler.hpp"
#include "CPPStructBuilder.hpp"

struct Args
{
    const char* shader = nullptr; // -s
    const char* entry  = nullptr; // -e
    const char* output = nullptr; // -o
    const char* stage  = nullptr; // -stage
    const char* target = nullptr; // -target
};

Args ParseArgs(int argc, const char** argv)
{
    Args args;
    for (int i = 1; i < argc; ++i)
    {
        if ((strcmp(argv[i], "-s") == 0 || strcmp(argv[i], "--shader") == 0) && i + 1 < argc)
            args.shader = argv[++i];
        else if ((strcmp(argv[i], "-e") == 0 || strcmp(argv[i], "--entry") == 0) && i + 1 < argc)
            args.entry = argv[++i];
        else if ((strcmp(argv[i], "-o") == 0 || strcmp(argv[i], "--output") == 0) && i + 1 < argc)
            args.output = argv[++i];
        else if ((strcmp(argv[i], "-stage") == 0) && i + 1 < argc)
            args.stage = argv[++i];
        else if ((strcmp(argv[i], "-target") == 0) && i + 1 < argc)
            args.target = argv[++i];
    }
    return args;
}

int main(int argc, const char** argv)
{
    Args args = ParseArgs(argc, argv);

    TL_LOG_INFO("Input arguments:");
    TL_LOG_INFO("  Shader: {}", args.shader ? args.shader : "(null)");
    TL_LOG_INFO("  Entry: {}", args.entry ? args.entry : "(null)");
    TL_LOG_INFO("  Output: {}", args.output ? args.output : "(null)");
    TL_LOG_INFO("  Stage: {}", args.stage ? args.stage : "(null)");
    TL_LOG_INFO("  Target: {}", args.target ? args.target : "(null)");

    // Call your shader compiler logic here
    // Example:
    // CompileShader(args.shader, args.entry, args.output, args.stage, args.target);

    using namespace CPPStructBuilder;

    ShaderCompiler c;
    auto a = c.CompileShader(args.shader);

    // // Simple struct
    // auto ta = Type::Create("Foo",
    //     {
    //         StructField{i32, "i" },
    //         StructField{f32, "j" },
    //         StructField{f64, "a" },
    //         StructField{u8,  "aa"},
    // });

    // // Another struct with different types
    // auto tb = Type::Create("Bar",
    //     {
    //         StructField{u32,     "x"   },
    //         StructField{boolean, "flag"},
    // });

    // // Nested struct: Baz contains Foo and Bar
    // auto tc = Type::Create("Baz",
    //     {
    //         StructField{ta,  "foo"  },
    //         StructField{tb,  "bar"  },
    //         StructField{i16, "count"},
    // });

    // // Print all definitions
    // TL_LOG_INFO("{}", ta->ToCppDefinition());
    // TL_LOG_INFO("{}", tb->ToCppDefinition());
    // TL_LOG_INFO("{}", tc->ToCppDefinition());

    // TL_LOG_INFO("{}", ta->ToCppDefinition());
    return 0;
}