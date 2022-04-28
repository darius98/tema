#include "compiler/parser.h"

#include "algorithms/print_utf8.h"

#include <iostream>

void print_module(const tema::module& mod) {
    for (const auto& decl: mod.get_decls()) {
        if (holds_alternative<tema::module::stmt_decl>(decl)) {
            const auto& stmt_decl = get<tema::module::stmt_decl>(decl);
            std::cout << stmt_decl.loc.file_name << ":" << stmt_decl.loc.line << ":" << stmt_decl.loc.col << " " << stmt_decl.name << ": ";
            tema::print_utf8_to(stmt_decl.stmt.get(), std::cout);
            std::cout << "\n";
        }
    }
}

int main() {
    try {
        const auto mod = tema::parse_module_from_file("src/modules/propositional_logic.tema");
        print_module(mod);
    } catch (tema::parse_error& err) {
        std::cout << err.what();
    }
    return 0;
}