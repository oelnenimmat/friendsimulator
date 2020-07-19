/*=============================================================================
Leo Tamminen
shophorn @ internet


Windows-Vulkan interface. And lots of rubbish at the moment.

STUDY: https://devblogs.nvidia.com/vulkan-dos-donts/
=============================================================================*/

#ifndef WIN_VULKAN_HPP

using VulkanContext = PlatformGraphics;

internal void 
print_vulkan_assert(LogInput::FileAddress address, VkResult result)
{
    logVulkan(0) << address << "Vulkan check failed " << vulkan::to_str(result) << "(" << result << ")\n";
}

#define VULKAN_CHECK(result) if (result != VK_SUCCESS) { print_vulkan_assert(FILE_ADDRESS, result); abort();}

// Note(Leo): Play with these later in development, when we have closer to final contents
// constexpr s32 VIRTUAL_FRAME_COUNT = 2;
constexpr s32 VIRTUAL_FRAME_COUNT = 3;


constexpr u64 VULKAN_NO_TIME_OUT = max_value_u64;
constexpr f32 VULKAN_MAX_LOD_FLOAT = 100.0f;

constexpr s32 VULKAN_MAX_MODEL_COUNT = 2000;
constexpr s32 VULKAN_MAX_MATERIAL_COUNT = 100;


constexpr VkSampleCountFlagBits VULKAN_MAX_MSAA_SAMPLE_COUNT = VK_SAMPLE_COUNT_2_BIT;


namespace vulkan
{
	// Note(Leo): these need to align properly
	// Study(Leo): these need to align properly
	struct CameraUniformBuffer
	{
		alignas(16) m44 view;
		alignas(16) m44 projection;
		alignas(16) m44 lightViewProjection;
		alignas(16) f32 shadowDistance;
		f32 shadowTransitionDistance;
	};

	struct LightingUniformBuffer
	{
		v4 direction;
		v4 color;
		v4 ambient;
		v4 cameraPosition;
		f32 skyColor;
	};
	
	struct ModelUniformBuffer
	{
		// Note(Leo): matrices must be aligned on 16 byte boundaries
		// Todo(Leo): Find the confirmation for this from Vulkan documentation
		alignas(16) m44 	localToWorld;
		alignas(16) float 	isAnimated;
		alignas(16) m44 	bonesToLocal [32];
	};
}

struct VulkanBufferResource
{
    VkBuffer buffer;
    VkDeviceMemory memory;
    
    // Todo[memory, vulkan](Leo): IMPORTANT Enforce/track these
    VkDeviceSize used;
    VkDeviceSize size;
};

// Todo(Leo): these should be local variables in initialization function
// struct VulkanQueueFamilyIndices
// struct VulkanSwapchainSupportDetails
struct VulkanQueueFamilyIndices
{
    u32 graphics;
    u32 present;

    bool32 hasGraphics;
    bool32 hasPresent;

    u32 getAt(int index)
    {
        if (index == 0) return graphics;
        if (index == 1) return present;
        return -1;
    }

    bool32 hasAll()
    {
        return hasGraphics && hasPresent;
    }
};

struct VulkanSwapchainSupportDetails
{
    VkSurfaceCapabilitiesKHR 		capabilities;
    std::vector<VkSurfaceFormatKHR> formats;
    std::vector<VkPresentModeKHR> 	presentModes;
};


struct VulkanTexture
{
	VkImage 		image;
	VkImageView 	view;
	VkSampler 		sampler;

	// TODO(Leo): totally not allocate like this, we need texture pool
	VkDeviceMemory memory;
};

struct VulkanGuiTexture
{
	VulkanTexture 	texture;
	VkDescriptorSet descriptorSet;
};

struct VulkanMesh
{
    VkBuffer        bufferReference;
    // VkDeviceMemory  memoryReference;

    VkDeviceSize    vertexOffset;
    VkDeviceSize    indexOffset;
    u32             indexCount;
    VkIndexType     indexType;
};

struct VulkanMaterial
{
	GraphicsPipeline 	pipeline;
	VkDescriptorSet 	descriptorSet;
};

/* Todo(Leo): this is now redundant it can go away.
Then we can simply draw by mesh and material anytime */
struct VulkanModel
{
	MeshHandle 		mesh;
	MaterialHandle 	material;
};

struct VulkanLoadedPipeline
{
	// Note(Leo): we need info for recreating pipelines after swapchain recreation
	VkPipeline 				pipeline;
	VkPipelineLayout 		layout;
	VkDescriptorSetLayout	materialLayout;
};

struct FSVulkanPipeline
{
	VkPipeline 				pipeline;
	VkPipelineLayout 		pipelineLayout;
	VkDescriptorSetLayout 	descriptorSetLayout;
	s32 					textureCount;
};

struct VulkanVirtualFrame
{
	struct
	{
		VkCommandBuffer master;
		VkCommandBuffer scene;
		VkCommandBuffer gui;

        VkCommandBuffer offscreen;

		// Todo(Leo): Do we want this too?
		// VkCommandBuffer debug;
	} commandBuffers;

	VkFramebuffer  framebuffer;
    VkFramebuffer  shadowFramebuffer;

    VkSemaphore 	shadowPassWaitSemaphore;
	VkSemaphore    imageAvailableSemaphore;
	VkSemaphore    renderFinishedSemaphore;
	VkFence        inFlightFence; // Todo(Leo): Change to queuesubmitfence or commandbufferfence etc..
};

struct PlatformGraphics
{
	VkInstance 						instance;
	VkDevice 						device; // Note(Leo): this is Logical Device.
	VkPhysicalDevice 				physicalDevice;
	VkPhysicalDeviceProperties 		physicalDeviceProperties;
	VkSurfaceKHR 					surface;

#if FS_VULKAN_USE_VALIDATION
	VkDebugUtilsMessengerEXT debugMessenger;
#endif

	struct {
		VkQueue graphics;
		VkQueue present;

		u32 graphicsIndex;
		u32 presentIndex;
	} queues;

    VulkanVirtualFrame virtualFrames [VIRTUAL_FRAME_COUNT];
    u32 virtualFrameIndex = 0;

    VkDescriptorPool 		uniformDescriptorPool;
    VkDescriptorPool 		materialDescriptorPool;

	// Note(Leo): these are not cleared on unload
    VkDescriptorPool		persistentDescriptorPool;


    VkDescriptorSetLayout 	modelDescriptorSetLayout;
    VkDescriptorSet 		modelDescriptorSet;

	VkDescriptorSetLayout 	cameraDescriptorSetLayout;
    VkDescriptorSet 		cameraDescriptorSet[VIRTUAL_FRAME_COUNT];
    
	VkDescriptorSetLayout 	lightingDescriptorSetLayout;
    VkDescriptorSet 		lightingDescriptorSet[VIRTUAL_FRAME_COUNT];
	
	VkDescriptorSetLayout 	shadowMapDescriptorSetLayout;
	VkDescriptorSet 		shadowMapDescriptorSet;

    // Todo(Leo): set these dynamically
    constexpr static VkDeviceSize cameraUniformOffset 			= 0;
    constexpr static VkDeviceSize lightingUniformOffset 		= 256;
    constexpr static VkDeviceSize sceneUniformBufferFrameOffset = 1024;

    static_assert(sizeof(vulkan::CameraUniformBuffer) <= 256);
    static_assert(sizeof(vulkan::LightingUniformBuffer) <= 256);

    // Uncategorized
	VkCommandPool 			commandPool;
    VkSampleCountFlagBits 	msaaSamples;
    VkSampler 				textureSampler;			
    VkSampler 				clampSampler;			

    /* Note(Leo): color and depth images for initial writing. These are
    afterwards resolved to actual framebufferimage */

	struct {
	    VkSwapchainKHR 	swapchain;
	    VkExtent2D 		extent;

	    VkFormat 					imageFormat;
	    std::vector<VkImage> 		images;
	    std::vector<VkImageView> 	imageViews;

	    VkRenderPass renderPass;

	    // Note(Leo): these are attchaments.
		VkDeviceMemory memory;

		VkImage 	colorImage;
		VkImageView colorImageView;

		VkImage 	depthImage;	
		VkImageView depthImageView;
	} drawingResources = {};


	struct
	{
		u32 width;
		u32 height;

		VulkanTexture 		attachment;
		VkSampler 			sampler; 

		VkRenderPass 		renderPass;
		VkFramebuffer 		framebuffer;

		VkPipelineLayout 	layout;
		VkPipeline 			pipeline;

	} shadowPass;

	// Todo(Leo): This is stupid, just use bool, we only have one use case, probably never have a ton, so just add more bools
	using PostRenderFunc = void(VulkanContext*);
	PostRenderFunc * onPostRender;

    VulkanBufferResource stagingBufferPool;
    VulkanBufferResource staticMeshPool;
    VulkanBufferResource modelUniformBuffer;
    VulkanBufferResource sceneUniformBuffer;

    // Todo(Leo): Use our own arena arrays for these.
    // Todo(Leo): That requires access to persistent memory block at platform layer
    // Todo(Leo): Also make these per "scene" thing, so we can unload stuff per scene easily
    std::vector<VulkanMesh>  	loadedMeshes;
	std::vector<VulkanTexture> 	loadedTextures;
	std::vector<VulkanMaterial>	loadedMaterials;
	std::vector<VulkanModel>	loadedModels;

	// Note(Leo): Guitextures are separeate becauses they are essentially texture AND material
	// since they are used alone. We should maybe just use normal materials. I don't know yet.
	std::vector<VulkanGuiTexture> 	loadedGuiTextures;
	std::vector<VulkanTexture> 		loadedGuiTexturesTextures;
	std::vector<VkDescriptorSet> 	loadedGuiTexturesDescriptorSets;

	VkDescriptorSet 		shadowMapTexture;

	FSVulkanPipeline pipelines [GRAPHICS_PIPELINE_COUNT];

	VkPipeline 				linePipeline;
	VkPipelineLayout 		linePipelineLayout;

	VkPipeline 				leavesShadowPipeline;
	VkPipelineLayout 		leavesShadowPipelineLayout;
	VkDescriptorSetLayout 	leavesShadowMaskDescriptorSetLayout;

	// Note(Leo): This is a list of functions to call when destroying this.
    using CleanupFunc = void (VulkanContext*);
	std::vector<CleanupFunc*> cleanups = {};

    u32 currentDrawFrameIndex;
    bool32 canDraw = false;

    bool32 sceneUnloaded = false;

    VkBuffer 		leafBuffer;
    VkDeviceMemory 	leafBufferMemory;
    VkDeviceSize 	leafBufferCapacity;
    VkDeviceSize 	leafBufferUsed[VIRTUAL_FRAME_COUNT];
    void * 			persistentMappedLeafBufferMemory;
};


// Note(Leo): We are expecting to at some point need to get things from multiple different
// containers, which is easier with helper function.
// Todo(Leo): any case, think through these at some point

internal VulkanGuiTexture & fsvulkan_get_loaded_gui_texture (VulkanContext & context, GuiTextureHandle id)
{
	return context.loadedGuiTextures[id];
}


// TODo(Leo): inline in render function
internal inline VulkanVirtualFrame *
fsvulkan_get_current_virtual_frame(VulkanContext * context)
{
	return &context->virtualFrames[context->virtualFrameIndex];
}

internal inline VulkanTexture *
fsvulkan_get_loaded_texture(VulkanContext * context, TextureHandle handle)
{
    return &context->loadedTextures[handle];
}

internal inline VulkanMaterial *
fsvulkan_get_loaded_material(VulkanContext * context, MaterialHandle handle)
{
	return &context->loadedMaterials[handle];
}

internal inline VulkanMesh *
fsvulkan_get_loaded_mesh(VulkanContext * context, MeshHandle handle)
{
	return &context->loadedMeshes[handle];
}

namespace vulkan
{
	// Todo(Leo): inline in init function
    internal VkFormat find_supported_depth_format(VkPhysicalDevice physicalDevice);
	internal u32 find_memory_type ( VkPhysicalDevice physicalDevice, u32 typeFilter, VkMemoryPropertyFlags properties);

	internal VulkanQueueFamilyIndices find_queue_families (VkPhysicalDevice device, VkSurfaceKHR surface);


#if FS_VULKAN_USE_VALIDATION
	constexpr bool32 enableValidationLayers = true;

	constexpr char const * const validationLayers[] =
	{
	    "VK_LAYER_KHRONOS_validation"
	};
	constexpr int VALIDATION_LAYERS_COUNT = array_count(validationLayers);

#else
	constexpr bool32 enableValidationLayers 	= false;

	constexpr char const ** validationLayers 	= nullptr;
	constexpr int VALIDATION_LAYERS_COUNT 		= 0;


#endif

	constexpr char const * const deviceExtensions [] =
	{
	    VK_KHR_SWAPCHAIN_EXTENSION_NAME,
	};
	constexpr int DEVICE_EXTENSION_COUNT = array_count(deviceExtensions);


	// Todo(Leo): This needs to be enforced
    constexpr s32 MAX_LOADED_TEXTURES = 100;

	/// INTERNAL RESOURCES, CUSTOM
	internal void destroy_texture 		(VulkanContext*, VulkanTexture * texture);

	/// INTERNAL RESOURCES, VULKAN TYPES
    internal VkRenderPass 			make_vk_render_pass(VulkanContext*, VkFormat format, VkSampleCountFlagBits msaaSamples);

	internal VkFramebuffer 			make_vk_framebuffer(VkDevice, VkRenderPass, u32 attachmentCount, VkImageView * attachments, u32 width, u32 height);
	internal VkImage 				make_vk_image(	VulkanContext*, u32 width, u32 height, u32 mipLevels, VkFormat format,
											    	VkImageTiling tiling, VkImageUsageFlags usage,
											    	VkSampleCountFlagBits msaaSamples);
	internal VkImageView 			make_vk_image_view(VkDevice device, VkImage image, u32 mipLevels, VkFormat format, VkImageAspectFlags aspectFlags);
	// internal VkSampler				make_vk_sampler(VkDevice, VkSamplerAddressMode);   
}

internal VkDescriptorSet make_material_vk_descriptor_set_2(	VulkanContext *			context,
															VkDescriptorSetLayout 	descriptorSetLayout,
															VkImageView 			imageView,
															VkDescriptorPool 		pool,
															VkSampler 				sampler,
															VkImageLayout 			layout);






#define WIN_VULKAN_HPP
#endif