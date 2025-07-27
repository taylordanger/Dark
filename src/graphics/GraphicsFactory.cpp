#include "IGraphicsAPI.h"
#include "MockGraphicsAPI.h"

#ifndef NO_GRAPHICS
#include "OpenGLAPI.h"
#endif

namespace RPGEngine {
namespace Graphics {

std::shared_ptr<IGraphicsAPI> createGraphicsAPI() {
#ifdef NO_GRAPHICS
    return std::make_shared<MockGraphicsAPI>();
#else
    return std::make_shared<OpenGLAPI>();
#endif
}

} // namespace Graphics
} // namespace RPGEngine