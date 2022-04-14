#pragma once

#include <memory>

#include "util/tpack.h"
#include "variable.h"

namespace tema {

struct expression : std::enable_shared_from_this<expression> {
    using expr_ptr = std::shared_ptr<const expression>;

    using types = util::tpack<variable_ptr>;

private:
    util::variant_for<types> data;

    // Ensure that expressions are only created through the friend factories below
    struct private_tag {};

public:
    expression(private_tag, util::one_of<types> auto t)
        : data(std::move(t)) {}

    expression(const expression&) = delete;
    expression& operator=(const expression&) = delete;
    expression(expression&&) = delete;
    expression& operator=(expression&&) = delete;
    ~expression() = default;

    [[nodiscard]] bool is_var() const noexcept;
    [[nodiscard]] variable_ptr as_var() const;

    template<class V>
    void accept(V&& visitor) const {
        std::visit(std::forward<V>(visitor), data);
    }

    template<class R, class V>
    R accept_r(V&& visitor) const {
        return std::visit<R>(std::forward<V>(visitor), data);
    }

    friend expr_ptr var_expr(variable_ptr var);
};

using expr_ptr = expression::expr_ptr;

[[nodiscard]] expr_ptr var_expr(variable_ptr var);

}// namespace tema
