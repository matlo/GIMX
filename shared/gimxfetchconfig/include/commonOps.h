#ifndef  COMMONOPS_H
#define  COMMONOPS_H

#include <string>
#include <list>
#include <functional>

#include <gimxinput/include/ginput.h>

#ifdef WIN32
#define REGISTER_FUNCTION gpoll_register_handle
#define REMOVE_FUNCTION gpoll_remove_handle
#else
#define REGISTER_FUNCTION gpoll_register_fd
#define REMOVE_FUNCTION gpoll_remove_fd
#endif


int process_cb(GE_Event* event __attribute__((unused)));

void autoConfigDownload(std::function<void(std::string)> func);


#endif //COMMONOPS_H
