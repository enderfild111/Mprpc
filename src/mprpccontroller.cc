#include "mprpccontroller.h"


MprpcController::MprpcController()
    :m_failed(false)
    ,m_errText("")
{

}

// 重置Controller
void MprpcController::Reset()
{
    m_failed = false;
    m_errText = "";
}

// RPC方法是否调用失败
bool MprpcController::Failed() const
{
    return m_failed;
}
// 调用失败的返回信息
std::string MprpcController::ErrorText() const
{
    return m_errText;
}
// 设置失败信息
void MprpcController::SetFailed(const std::string& reason)
{
    m_failed = true;
    m_errText = reason;
}
