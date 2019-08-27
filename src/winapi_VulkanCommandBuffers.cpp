/*=============================================================================
Leo Tamminen

This file implements common Vulkan command buffer operations
=============================================================================*/

namespace vulkan
{
    internal VkCommandBuffer
    BeginOneTimeCommandBuffer(
        VkDevice           logicalDevice,
        VkCommandPool      commandPool);

    internal void
    EndOneTimeCommandBuffer(
    	VkDevice           logicalDevice,
    	VkCommandPool      commandPool,
    	VkQueue            submitQueue,
    	VkCommandBuffer    commandBuffer);
}

internal VkCommandBuffer
vulkan::BeginOneTimeCommandBuffer(VkDevice logicalDevice, VkCommandPool commandPool)
{
    VkCommandBufferAllocateInfo allocateInfo = { VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO };
    allocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;

    allocateInfo.commandPool = commandPool;
    allocateInfo.commandBufferCount = 1;

    VkCommandBuffer commandBuffer;
    vkAllocateCommandBuffers(logicalDevice, &allocateInfo, &commandBuffer);

    VkCommandBufferBeginInfo beginInfo = { VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO };
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

    vkBeginCommandBuffer(commandBuffer, &beginInfo);

    return commandBuffer;
}

internal void
vulkan::EndOneTimeCommandBuffer(
    VkDevice logicalDevice,
    VkCommandPool commandPool,
    VkQueue submitQueue,
    VkCommandBuffer commandBuffer
){
    vkEndCommandBuffer(commandBuffer);

    VkSubmitInfo submitInfo = { VK_STRUCTURE_TYPE_SUBMIT_INFO };
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &commandBuffer;

    vkQueueSubmit(submitQueue, 1,  &submitInfo, VK_NULL_HANDLE);
    
    // Todo(Leo): Mayube we should use VkFence and vkWaitForFences here instead of wait idle
    // Note(Leo): graphics and compute queues support transfer operations implicitly
    vkQueueWaitIdle(submitQueue);

    vkFreeCommandBuffers(logicalDevice, commandPool, 1, &commandBuffer);
}