#include <stdint.h>

#include "kentry.h"
#include "kernel.h"

#include <stdio.h>
#include <stdlib.h>

void* __gxx_personality_v0 = 0;
void* _Unwind_Resume = 0;

extern "C" void __dso_handle() {}
extern "C" void __cxa_atexit() {}
extern "C" void __cxa_pure_virtual() {}

#include <stdlib.h>
#include <vector.h>

#include "int/pit.h"

void kmain(bamboo_boot_info_t* boot_info) {

    kernel::init(boot_info);

    // io::printf("The following notice and this permission notice shall be included in all\n");
    // io::printf("copies or substantial portions of BambooOS (the \"Software\").\n\n");

    // io::printf("THE SOFTWARE IS PROVIDED \"AS IS\", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED,\n");
    // io::printf("INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A\n");
    // io::printf("PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT\n");
    // io::printf("HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF\n");
    // io::printf("CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE\n");
    // io::printf("OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.\n\n");
    
    io::printf("> Hello, world!\n");

    while(1) {
        asm("hlt");
    }
}

void main(bamboo_boot_info_t* boot_info) {
    kmain(boot_info);
}
