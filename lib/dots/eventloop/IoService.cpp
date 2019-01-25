#include "IoService.h"

namespace dots {

IoService &ioService()
{
    static IoService *io_service = new IoService;
    return *io_service;
}

}