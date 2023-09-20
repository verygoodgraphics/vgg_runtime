#ifndef VGG_SDK_ADDON_HPP
#define VGG_SDK_ADDON_HPP

namespace node
{
class Environment;
}

void link_vgg_sdk_addon(node::Environment* env);

#endif