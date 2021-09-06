// This file is licensed under the Elastic License 2.0. Copyright 2021 StarRocks Limited.

#include "exprs/vectorized/arithmetic_expr.h"

#include <glog/logging.h>
#include <gtest/gtest.h>

#include "butil/time.h"
#include "column/fixed_length_column.h"
#include "exprs/arithmetic_expr.h"
#include "exprs/vectorized/mock_vectorized_expr.h"

namespace starrocks {
namespace vectorized {

class VectorizedArithmeticExprTest : public ::testing::Test {
public:
    void SetUp() {
        expr_node.opcode = TExprOpcode::ADD;
        expr_node.child_type = TPrimitiveType::INT;
        expr_node.node_type = TExprNodeType::BINARY_PRED;
        expr_node.num_children = 2;
        expr_node.__isset.opcode = true;
        expr_node.__isset.child_type = true;
        expr_node.type = gen_type_desc(TPrimitiveType::INT);
    }

public:
    TExprNode expr_node;
};

TEST_F(VectorizedArithmeticExprTest, addExpr) {
    // normal int8
    {
        expr_node.opcode = TExprOpcode::ADD;
        expr_node.type = gen_type_desc(TPrimitiveType::TINYINT);

        std::unique_ptr<Expr> expr(VectorizedArithmeticExprFactory::from_thrift(expr_node));

        MockVectorizedExpr<TYPE_TINYINT> col1(expr_node, 10, 1);
        MockVectorizedExpr<TYPE_TINYINT> col2(expr_node, 10, 2);

        expr->_children.push_back(&col1);
        expr->_children.push_back(&col2);

        ColumnPtr ptr = expr->evaluate(nullptr, nullptr);

        ASSERT_FALSE(ptr->is_nullable());
        ASSERT_TRUE(ptr->is_numeric());

        auto v = std::static_pointer_cast<Int8Column>(ptr);
        ASSERT_EQ(10, v->size());

        for (int j = 0; j < v->size(); ++j) {
            ASSERT_EQ(3, v->get_data()[j]);
        }
    }

    // normal int
    {
        expr_node.opcode = TExprOpcode::ADD;
        expr_node.type = gen_type_desc(TPrimitiveType::INT);

        std::unique_ptr<Expr> expr(VectorizedArithmeticExprFactory::from_thrift(expr_node));

        MockVectorizedExpr<TYPE_INT> col1(expr_node, 10, 1);
        MockVectorizedExpr<TYPE_INT> col2(expr_node, 10, 2);

        expr->_children.push_back(&col1);
        expr->_children.push_back(&col2);

        ColumnPtr ptr = expr->evaluate(nullptr, nullptr);

        ASSERT_FALSE(ptr->is_nullable());
        ASSERT_TRUE(ptr->is_numeric());

        auto v = std::static_pointer_cast<Int32Column>(ptr);
        ASSERT_EQ(10, v->size());

        for (int j = 0; j < v->size(); ++j) {
            ASSERT_EQ(3, v->get_data()[j]);
        }
    }

    {
        expr_node.opcode = TExprOpcode::ADD;
        expr_node.type = gen_type_desc(TPrimitiveType::FLOAT);

        std::unique_ptr<Expr> expr(VectorizedArithmeticExprFactory::from_thrift(expr_node));

        MockVectorizedExpr<TYPE_FLOAT> col1(expr_node, 10, 1);
        MockVectorizedExpr<TYPE_FLOAT> col2(expr_node, 10, 2);

        expr->_children.push_back(&col1);
        expr->_children.push_back(&col2);

        ColumnPtr ptr = expr->evaluate(nullptr, nullptr);

        ASSERT_FALSE(ptr->is_nullable());
        ASSERT_TRUE(ptr->is_numeric());

        auto v = std::static_pointer_cast<FloatColumn>(ptr);
        ASSERT_EQ(10, v->size());

        for (int j = 0; j < v->size(); ++j) {
            ASSERT_EQ(3, v->get_data()[j]);
        }
    }
}

TEST_F(VectorizedArithmeticExprTest, mulExpr) {
    expr_node.opcode = TExprOpcode::MULTIPLY;
    std::unique_ptr<Expr> expr(VectorizedArithmeticExprFactory::from_thrift(expr_node));

    MockVectorizedExpr<TYPE_INT> col1(expr_node, 10, 10);
    MockVectorizedExpr<TYPE_INT> col2(expr_node, 10, 2);

    expr->_children.push_back(&col1);
    expr->_children.push_back(&col2);

    {
        ColumnPtr ptr = expr->evaluate(nullptr, nullptr);

        auto v = std::static_pointer_cast<Int32Column>(ptr);
        ASSERT_EQ(10, v->size());

        for (int j = 0; j < v->size(); ++j) {
            ASSERT_EQ(20, v->get_data()[j]);
        }
    }
}

TEST_F(VectorizedArithmeticExprTest, nullMulExpr) {
    expr_node.opcode = TExprOpcode::MULTIPLY;
    std::unique_ptr<Expr> expr(VectorizedArithmeticExprFactory::from_thrift(expr_node));

    MockNullVectorizedExpr<TYPE_INT> col1(expr_node, 10, 10);
    MockNullVectorizedExpr<TYPE_INT> col2(expr_node, 10, 2);
    ++col2.flag;

    expr->_children.push_back(&col1);
    expr->_children.push_back(&col2);

    {
        ColumnPtr v = col1.evaluate(nullptr, nullptr);
        ASSERT_TRUE(v->is_nullable());
        ASSERT_EQ(10, v->size());

        for (int j = 0; j < v->size(); ++j) {
            if (j % 2) {
                ASSERT_TRUE(v->is_null(j));
            } else {
                ASSERT_FALSE(v->is_null(j));
            }
        }

        auto ptr = std::static_pointer_cast<NullableColumn>(v)->data_column();
        for (int j = 0; j < v->size(); ++j) {
            ASSERT_EQ(10, std::static_pointer_cast<Int32Column>(ptr)->get_data()[j]);
        }
    }

    {
        ColumnPtr v = col2.evaluate(nullptr, nullptr);
        ASSERT_TRUE(v->is_nullable());
        ASSERT_EQ(10, v->size());

        for (int j = 0; j < v->size(); ++j) {
            if (j % 2) {
                ASSERT_FALSE(v->is_null(j));
            } else {
                ASSERT_TRUE(v->is_null(j));
            }
        }
    }
    {
        ColumnPtr ptr = expr->evaluate(nullptr, nullptr);

        ASSERT_TRUE(ptr->is_nullable());
        ASSERT_FALSE(ptr->is_numeric());

        auto v = std::static_pointer_cast<NullableColumn>(ptr);
        ASSERT_EQ(10, v->size());

        for (int j = 0; j < v->size(); ++j) {
            ASSERT_TRUE(v->is_null(j));
        }
    }
}

TEST_F(VectorizedArithmeticExprTest, divExpr) {
    expr_node.opcode = TExprOpcode::DIVIDE;
    std::unique_ptr<Expr> expr(VectorizedArithmeticExprFactory::from_thrift(expr_node));

    MockVectorizedExpr<TYPE_INT> col1(expr_node, 10, 10);
    MockVectorizedExpr<TYPE_INT> col2(expr_node, 10, 2);

    expr->_children.push_back(&col1);
    expr->_children.push_back(&col2);

    {
        ColumnPtr ptr = expr->evaluate(nullptr, nullptr);

        auto v = std::static_pointer_cast<NullableColumn>(ptr);
        ASSERT_TRUE(v->is_nullable());
        ASSERT_EQ(10, v->size());

        auto nums = std::static_pointer_cast<Int32Column>(v->data_column());

        for (int j = 0; j < nums->size(); ++j) {
            ASSERT_EQ(5, nums->get_data()[j]);
        }

        for (int j = 0; j < nums->size(); ++j) {
            ASSERT_FALSE(v->is_null(j));
        }
    }
}

TEST_F(VectorizedArithmeticExprTest, produceNullModExpr) {
    expr_node.opcode = TExprOpcode::MOD;
    std::unique_ptr<Expr> expr(VectorizedArithmeticExprFactory::from_thrift(expr_node));

    MockVectorizedExpr<TYPE_INT> col1(expr_node, 10, 10);
    MockVectorizedExpr<TYPE_INT> col2(expr_node, 10, 0);

    expr->_children.push_back(&col1);
    expr->_children.push_back(&col2);

    {
        ColumnPtr v = col1.evaluate(nullptr, nullptr);
        ASSERT_FALSE(v->is_nullable());

        for (int j = 0; j < v->size(); ++j) {
            ASSERT_FALSE(v->is_null(j));
        }
    }

    {
        ColumnPtr ptr = expr->evaluate(nullptr, nullptr);

        ASSERT_TRUE(ptr->is_nullable());
        ASSERT_FALSE(ptr->is_numeric());

        auto v = std::static_pointer_cast<NullableColumn>(ptr);
        ASSERT_EQ(10, v->size());

        for (int j = 0; j < v->size(); ++j) {
            ASSERT_EQ(0, std::static_pointer_cast<Int32Column>(v->data_column())->get_data()[j]);
        }

        for (int j = 0; j < v->size(); ++j) {
            ASSERT_TRUE(v->is_null(j));
        }
    }
}

TEST_F(VectorizedArithmeticExprTest, mergeNullDivExpr) {
    expr_node.opcode = TExprOpcode::DIVIDE;
    std::unique_ptr<Expr> expr(VectorizedArithmeticExprFactory::from_thrift(expr_node));

    MockNullVectorizedExpr<TYPE_INT> col1(expr_node, 10, 10);
    MockNullVectorizedExpr<TYPE_INT> col2(expr_node, 10, 2);
    ++col2.flag;

    expr->_children.push_back(&col1);
    expr->_children.push_back(&col2);

    {
        ColumnPtr v = col1.evaluate(nullptr, nullptr);
        ASSERT_TRUE(v->is_nullable());

        for (int j = 0; j < v->size(); ++j) {
            for (int j = 0; j < v->size(); ++j) {
                if (j % 2) {
                    ASSERT_TRUE(v->is_null(j));
                } else {
                    ASSERT_FALSE(v->is_null(j));
                }
            }
        }
    }

    {
        ColumnPtr v = col2.evaluate(nullptr, nullptr);
        ASSERT_TRUE(v->is_nullable());

        for (int j = 0; j < v->size(); ++j) {
            for (int j = 0; j < v->size(); ++j) {
                if (j % 2) {
                    ASSERT_FALSE(v->is_null(j));
                } else {
                    ASSERT_TRUE(v->is_null(j));
                }
            }
        }
    }

    {
        ColumnPtr ptr = expr->evaluate(nullptr, nullptr);

        ASSERT_TRUE(ptr->is_nullable());
        ASSERT_FALSE(ptr->is_numeric());

        auto v = std::static_pointer_cast<NullableColumn>(ptr);
        ASSERT_EQ(10, v->size());

        for (int j = 0; j < v->size(); ++j) {
            ASSERT_EQ(5, std::static_pointer_cast<Int32Column>(v->data_column())->get_data()[j]);
        }

        for (int j = 0; j < v->size(); ++j) {
            ASSERT_TRUE(v->is_null(j));
        }
    }
}

TEST_F(VectorizedArithmeticExprTest, constVectorMulExpr) {
    expr_node.opcode = TExprOpcode::MULTIPLY;
    std::unique_ptr<Expr> expr(VectorizedArithmeticExprFactory::from_thrift(expr_node));

    MockConstVectorizedExpr<TYPE_INT> col1(expr_node, 10);
    MockVectorizedExpr<TYPE_INT> col2(expr_node, 10, 5);

    expr->_children.push_back(&col1);
    expr->_children.push_back(&col2);

    {
        ColumnPtr ptr = expr->evaluate(nullptr, nullptr);

        auto v = std::static_pointer_cast<Int32Column>(ptr);
        ASSERT_EQ(10, v->size());

        for (int j = 0; j < v->size(); ++j) {
            ASSERT_EQ(50, v->get_data()[j]);
        }
    }
}

TEST_F(VectorizedArithmeticExprTest, constConstAddExpr) {
    expr_node.opcode = TExprOpcode::ADD;
    std::unique_ptr<Expr> expr(VectorizedArithmeticExprFactory::from_thrift(expr_node));

    MockConstVectorizedExpr<TYPE_INT> col1(expr_node, 10);
    MockConstVectorizedExpr<TYPE_INT> col2(expr_node, 3);

    expr->_children.push_back(&col1);
    expr->_children.push_back(&col2);

    {
        ColumnPtr ptr = expr->evaluate(nullptr, nullptr);

        ASSERT_TRUE(ptr->is_constant());

        auto v = std::static_pointer_cast<ConstColumn>(ptr)->data_column();
        ASSERT_EQ(1, v->size());

        for (int j = 0; j < v->size(); ++j) {
            ASSERT_EQ(13, std::static_pointer_cast<Int32Column>(v)->get_data()[j]);
        }
    }
}

TEST_F(VectorizedArithmeticExprTest, produceNullVecotConstModExpr) {
    expr_node.opcode = TExprOpcode::MOD;
    std::unique_ptr<Expr> expr(VectorizedArithmeticExprFactory::from_thrift(expr_node));

    MockVectorizedExpr<TYPE_INT> col1(expr_node, 10, 10);
    MockConstVectorizedExpr<TYPE_INT> col2(expr_node, 0);

    expr->_children.push_back(&col1);
    expr->_children.push_back(&col2);

    {
        ColumnPtr v = col1.evaluate(nullptr, nullptr);
        ASSERT_FALSE(v->is_nullable());

        for (int j = 0; j < v->size(); ++j) {
            ASSERT_FALSE(v->is_null(j));
        }

        ColumnPtr v2 = col2.evaluate(nullptr, nullptr);
        ASSERT_FALSE(v2->is_nullable());
        ASSERT_TRUE(v2->is_constant());
        ASSERT_EQ(1, v2->size());
    }

    {
        ColumnPtr ptr = expr->evaluate(nullptr, nullptr);

        ASSERT_TRUE(ptr->is_nullable());
        ASSERT_FALSE(ptr->is_numeric());

        auto v = std::static_pointer_cast<NullableColumn>(ptr);
        ASSERT_EQ(10, v->size());

        for (int j = 0; j < v->size(); ++j) {
            ASSERT_EQ(0, std::static_pointer_cast<Int32Column>(v->data_column())->get_data()[j]);
        }

        for (int j = 0; j < v->size(); ++j) {
            ASSERT_TRUE(v->is_null(j));
        }
    }
}

TEST_F(VectorizedArithmeticExprTest, bitNotExpr) {
    expr_node.opcode = TExprOpcode::BITNOT;
    std::unique_ptr<Expr> expr(VectorizedArithmeticExprFactory::from_thrift(expr_node));

    MockVectorizedExpr<TYPE_INT> col1(expr_node, 10, 1);

    {
        expr->_children.push_back(&col1);
        ColumnPtr ptr = expr->evaluate(nullptr, nullptr);

        auto v = std::static_pointer_cast<Int32Column>(ptr);

        for (int j = 0; j < ptr->size(); ++j) {
            ASSERT_EQ(~1, v->get_data()[j]);
        }

        for (int j = 0; j < ptr->size(); ++j) {
            ASSERT_FALSE(v->is_null(j));
        }
    }
}

TEST_F(VectorizedArithmeticExprTest, constBitNotExpr) {
    expr_node.opcode = TExprOpcode::BITNOT;
    std::unique_ptr<Expr> expr(VectorizedArithmeticExprFactory::from_thrift(expr_node));

    MockConstVectorizedExpr<TYPE_INT> col1(expr_node, 2);

    {
        expr->_children.push_back(&col1);
        ColumnPtr ptr = expr->evaluate(nullptr, nullptr);
        ASSERT_TRUE(ptr->is_constant());

        auto re = std::static_pointer_cast<ConstColumn>(ptr);
        auto v = std::static_pointer_cast<Int32Column>(re->data_column());

        for (int j = 0; j < ptr->size(); ++j) {
            ASSERT_EQ(~2, v->get_data()[j]);
        }

        for (int j = 0; j < ptr->size(); ++j) {
            ASSERT_FALSE(v->is_null(j));
        }
    }
}

TEST_F(VectorizedArithmeticExprTest, constModExpr) {
    expr_node.opcode = TExprOpcode::MOD;
    expr_node.child_type = TPrimitiveType::BIGINT;
    expr_node.type = gen_type_desc(TPrimitiveType::BIGINT);

    std::unique_ptr<Expr> expr(VectorizedArithmeticExprFactory::from_thrift(expr_node));

    MockConstVectorizedExpr<TYPE_BIGINT> col1(expr_node, INT64_MIN);
    MockConstVectorizedExpr<TYPE_BIGINT> col2(expr_node, -1);

    expr->_children.push_back(&col1);
    expr->_children.push_back(&col2);

    {
        ColumnPtr v = col1.evaluate(nullptr, nullptr);
        ASSERT_FALSE(v->is_nullable());

        for (int j = 0; j < v->size(); ++j) {
            ASSERT_FALSE(v->is_null(j));
        }

        ColumnPtr v2 = col2.evaluate(nullptr, nullptr);
        ASSERT_FALSE(v2->is_nullable());
        ASSERT_TRUE(v2->is_constant());
        ASSERT_EQ(1, v2->size());
    }

    {
        ColumnPtr ptr = expr->evaluate(nullptr, nullptr);

        ASSERT_FALSE(ptr->is_nullable());
        ASSERT_TRUE(ptr->is_constant());
        ASSERT_FALSE(ptr->is_numeric());

        auto v = std::static_pointer_cast<ConstColumn>(ptr);
        ASSERT_EQ(1, v->size());

        for (int j = 0; j < v->size(); ++j) {
            ASSERT_EQ(0, ColumnHelper::cast_to_raw<TYPE_BIGINT>(v->data_column())->get_data()[j]);
        }

        for (int j = 0; j < v->size(); ++j) {
            ASSERT_FALSE(v->is_null(j));
        }
    }
}

TEST_F(VectorizedArithmeticExprTest, constMod128Expr) {
    expr_node.opcode = TExprOpcode::MOD;
    expr_node.child_type = TPrimitiveType::LARGEINT;
    expr_node.type = gen_type_desc(TPrimitiveType::LARGEINT);

    std::unique_ptr<Expr> expr(VectorizedArithmeticExprFactory::from_thrift(expr_node));

    MockConstVectorizedExpr<TYPE_LARGEINT> col1(expr_node, MIN_INT128);
    MockConstVectorizedExpr<TYPE_LARGEINT> col2(expr_node, -1);

    expr->_children.push_back(&col1);
    expr->_children.push_back(&col2);

    {
        ColumnPtr v = col1.evaluate(nullptr, nullptr);
        ASSERT_FALSE(v->is_nullable());

        for (int j = 0; j < v->size(); ++j) {
            ASSERT_FALSE(v->is_null(j));
        }

        ColumnPtr v2 = col2.evaluate(nullptr, nullptr);
        ASSERT_FALSE(v2->is_nullable());
        ASSERT_TRUE(v2->is_constant());
        ASSERT_EQ(1, v2->size());
    }

    {
        ColumnPtr ptr = expr->evaluate(nullptr, nullptr);

        ASSERT_FALSE(ptr->is_nullable());
        ASSERT_TRUE(ptr->is_constant());
        ASSERT_FALSE(ptr->is_numeric());

        auto v = std::static_pointer_cast<ConstColumn>(ptr);
        ASSERT_EQ(1, v->size());

        for (int j = 0; j < v->size(); ++j) {
            ASSERT_EQ(0, ColumnHelper::cast_to_raw<TYPE_LARGEINT>(v->data_column())->get_data()[j]);
        }

        for (int j = 0; j < v->size(); ++j) {
            ASSERT_FALSE(v->is_null(j));
        }
    }
}

TEST_F(VectorizedArithmeticExprTest, constModN128Expr) {
    expr_node.opcode = TExprOpcode::MOD;
    expr_node.child_type = TPrimitiveType::LARGEINT;
    expr_node.type = gen_type_desc(TPrimitiveType::LARGEINT);

    std::unique_ptr<Expr> expr(VectorizedArithmeticExprFactory::from_thrift(expr_node));

    MockConstVectorizedExpr<TYPE_LARGEINT> col1(expr_node, MAX_INT128);
    MockConstVectorizedExpr<TYPE_LARGEINT> col2(expr_node, -1);

    expr->_children.push_back(&col1);
    expr->_children.push_back(&col2);

    {
        ColumnPtr v = col1.evaluate(nullptr, nullptr);
        ASSERT_FALSE(v->is_nullable());

        for (int j = 0; j < v->size(); ++j) {
            ASSERT_FALSE(v->is_null(j));
        }

        ColumnPtr v2 = col2.evaluate(nullptr, nullptr);
        ASSERT_FALSE(v2->is_nullable());
        ASSERT_TRUE(v2->is_constant());
        ASSERT_EQ(1, v2->size());
    }

    {
        ColumnPtr ptr = expr->evaluate(nullptr, nullptr);

        ASSERT_FALSE(ptr->is_nullable());
        ASSERT_TRUE(ptr->is_constant());
        ASSERT_FALSE(ptr->is_numeric());

        auto v = std::static_pointer_cast<ConstColumn>(ptr);
        ASSERT_EQ(1, v->size());

        for (int j = 0; j < v->size(); ++j) {
            ASSERT_EQ(0, ColumnHelper::cast_to_raw<TYPE_LARGEINT>(v->data_column())->get_data()[j]);
        }

        for (int j = 0; j < v->size(); ++j) {
            ASSERT_FALSE(v->is_null(j));
        }
    }
}

} // namespace vectorized
} // namespace starrocks