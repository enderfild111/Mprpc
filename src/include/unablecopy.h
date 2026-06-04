#pragma once

class UnableCopy
{
public:
    UnableCopy() = default;
    UnableCopy(const UnableCopy&) = delete;
    UnableCopy& operator=(const UnableCopy&) = delete;
    UnableCopy(UnableCopy&&) = delete;
    UnableCopy& operator=(UnableCopy&&) = delete;
};