// TODO: Add tests!

/*
// TODO: Add back these test!

test("parse from file", [] {
    mcga::test::temp_file temp_file{"tema"};
    temp_file.writer << R"(
var p
export var q
definition "Truth-definition" ⊤
theorem "Truth-theorem" ⊤ proof missing
exercise "Truth-exercise" ⊤ proof missing
)";
    temp_file.writer.flush();
    auto mod = parse_module(temp_file.file_path);
    expect(mod.get_decls(), hasSize(5));
});

test("parse from non-existent file", [] {
    mcga::test::expect([] {
        std::filesystem::path file_name = mcga::test::make_temp_file_name("tema");
        (void) parse_module(file_name);
    },
                       throwsA<parse_error>);
});
 */

