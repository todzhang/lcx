#include <gtest/gtest.h>
#include <gmock/gmock.h>

//引用被测函数的头文件
extern "C"
{
    #include "code_in_c.h"
}

using namespace testing;
/*构造mock*/
//定义mock类
class Mock_FOO {
public:
    //定义mock方法
    MOCK_METHOD1(set, int(int num));
    //定义mock方法
    MOCK_METHOD0(get, int());
    //定义mock方法
    MOCK_METHOD1(mock_c_func, int(c_struct_t* p_c_struct));
};
//实例化mock对象
Mock_FOO mocker;
//创建mock对象方法的函数的C包装
int mock_c_func(c_struct_t* p_c_struct) {
    return mocker.mock_c_func(p_c_struct);
}

//测试被测函数
TEST(Atest, test_c_func_000)
{
    EXPECT_CALL(mocker, mock_c_func(IsNull())).WillRepeatedly(Return(654));

    c_struct_t c_struct_foo;
    c_struct_foo.p_c_func = mock_c_func;
    int ret = c_func_caller(&c_struct_foo);
    EXPECT_EQ(456, ret);
}

//测试被测函数
TEST(Atest, test_c_func_001)
{
    EXPECT_CALL(mocker, mock_c_func(An<c_struct_t*>())).WillRepeatedly(Return(654));

    c_struct_t c_struct_foo;
    c_struct_foo.p_c_func = mock_c_func;
    int l = c_func_caller(&c_struct_foo);
    EXPECT_EQ(654, l);
}

/*
int main(int argc, char *argv[]) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}*/
