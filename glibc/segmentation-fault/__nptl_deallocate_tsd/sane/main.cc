#include <cassert>
#include <iostream>
#include <thread>
#include <sane/sane.h>

void scan_thread() {
    SANE_Status status;

    status = sane_init(nullptr, nullptr);
    assert(status == SANE_STATUS_GOOD);

    const SANE_Device** device_list = nullptr;
    status = sane_get_devices(&device_list, false);
    assert(status == SANE_STATUS_GOOD);

    for(int i = 0; device_list[i] != nullptr; ++i){
        std::cout << device_list[i]->name << std::endl;
    }

    sane_exit();
}

int main() {
    std::thread t(scan_thread);
    t.join();
    return 0;
}
