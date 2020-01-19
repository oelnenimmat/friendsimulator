/*=============================================================================
Leo Tamminen
shophorn @ internet


Windows-Vulkan interface. And lots of rubbish at the moment.

STUDY: https://devblogs.nvidia.com/vulkan-dos-donts/
=============================================================================*/

#ifndef WIN_VULKAN_HPP

using string_literal = char const *;

internal void 
print_vulkan_assert(string_literal file, s32 line, VkResult result)
{
    std::cout << "Vulkan check failed [" << file << ":" << line << "]: " << vulkan::to_str(result) << "(" << result << ")\n";
}

#define VULKAN_CHECK(result) if (result != VK_SUCCESS) { print_vulkan_assert(__FILE__, __LINE__, result); abort();}


constexpr s32 VIRTUAL_FRAME_COUNT = 3;


constexpr u64 VULKAN_NO_TIME_OUT	= maxValue<u64>;
constexpr real32 VULKAN_MAX_LOD_FLOAT = 100.0f;

constexpr s32 VULKAN_MAX_MODEL_COUNT = 2000;
constexpr s32 VULKAN_MAX_MATERIAL_COUNT = 100;


constexpr VkSampleCountFlagBits VULKAN_MAX_MSAA_SAMPLE_COUNT = VK_SAMPLE_COUNT_1_BIT;

// Note(Leo): these need to align properly
struct VulkanCameraUniformBufferObject
{
	alignas(16) Matrix44 view;
	alignas(16) Matrix44 perspective;
};

// Todo[rendering] (Leo): Use this VulkanModelUniformBufferObject
// struct VulkanModelUniformBufferObject
// {
// 	alignas(16) Matrix44 modelMatrix;
// };



// Todo (Leo): important this is used like an memory arena somewhere, and like somthing else elsewhere.
struct VulkanBufferResource
{
    VkBuffer buffer;
    VkDeviceMemory memory;
    
    // Todo[memory, vulkan](Leo): IMPORTANT Enforce/track these
    u64 used;
    u64 size;
};

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
    VkSurfaceCapabilitiesKHR capabilities;
    std::vector<VkSurfaceFormatKHR> formats;
    std::vector<VkPresentModeKHR> presentModes;
};


struct VulkanTexture
{
	VkImage 		image;
	VkImageView 	view;

	// TODO(Leo): totally not allocate like this, we need texture pool
	VkDeviceMemory memory;
};

struct VulkanMesh
{
    VkBuffer        buffer;
    VkDeviceMemory  memory;

    VkDeviceSize    vertexOffset;
    VkDeviceSize    indexOffset;
    u32             indexCount;
    VkIndexType     indexType;
};

struct VulkanMaterial
{
	PipelineHandle pipeline;
	VkDescriptorSet descriptorSet;
};

/* Todo(Leo): this is now redundant it can go away.
Then we can simply draw by mesh and material anytime */
struct VulkanModel
{
	MeshHandle 		mesh;
	MaterialHandle 	material;
};

struct VulkanPipelineLoadInfo
{
	std::string vertexShaderPath;
	std::string fragmentShaderPath;
	platform::RenderingOptions options;
};

struct VulkanLoadedPipeline
{
	// Note(Leo): we need info for recreating pipelines after swapchain recreation
	VulkanPipelineLoadInfo 	info;
	VkPipeline 				pipeline;
	VkPipelineLayout 		layout;
	VkDescriptorSetLayout	materialLayout;
};

// struct VulkanOffscreenPass
// {
//     u32 width;
//     u32 height;
    
//     VkFramebuffer framebuffer;
//     struct
//     {
//         VulkanTexture color;
//         VulkanTexture depth;
//     } attachments;

//     VkRenderPass renderPass;
//     VkSampler sampler;
//     VkDescriptorImageInfo descriptor;
// };

struct VulkanVirtualFrame
{
	struct
	{
		VkCommandBuffer primary;
		VkCommandBuffer scene;
		VkCommandBuffer gui;

        VkCommandBuffer offscreen;

		// Todo(Leo): Do we want this too?
		// VkCommandBuffer debug;
	} commandBuffers;

	VkFramebuffer  framebuffer;
    VkFramebuffer  bonusFramebuffer;

	VkSemaphore    imageAvailableSemaphore;
	VkSemaphore    renderFinishedSemaphore;
	VkFence        inFlightFence; // Todo(Leo): Change to queuesubmitfence or commandbufferfence etc..
};

struct platform::Graphics
{
	VkInstance 						instance;
	VkDevice 						device; // Note(Leo): this is Logical Device.
	VkPhysicalDevice 				physicalDevice;
	VkPhysicalDeviceProperties 		physicalDeviceProperties;
	VkSurfaceKHR 					surface;

	struct {
		VkQueue graphics;
		VkQueue present;
	} queues;



    VulkanVirtualFrame virtualFrames [VIRTUAL_FRAME_COUNT];
    u32 virtualFrameIndex = 0;

    struct
    {
    	VkDescriptorSetLayout scene;
    	VkDescriptorSetLayout model;
    } descriptorSetLayouts;

    // VkDescriptorPool uniformDescriptorPool;
    // VkDescriptorPool materialDescriptorPool;

    // VkDescriptorPool persistentDescriptorPool;

    struct
    {
    	VkDescriptorPool uniform;
    	VkDescriptorPool material;

    	VkDescriptorPool persistent;
    } descriptorPools;

    struct
    {
	    VkDescriptorSet model;
   	 	VkDescriptorSet scene;
    } uniformDescriptorSets;


    // Uncategorized
	VkCommandPool 			commandPool;
    VkSampleCountFlagBits 	msaaSamples;
    VkSampler 				textureSampler;			

    /* Note(Leo): color and depth images for initial writing. These are
    afterwards resolved to actual framebufferimage */

	struct {
	    VkSwapchainKHR swapchain;
	    VkExtent2D extent;

	    VkFormat imageFormat;
	    std::vector<VkImage> images;
	    std::vector<VkImageView> imageViews;

	    VkRenderPass            renderPass;

	    // Note(Leo): these are attchaments.
		VkDeviceMemory memory;

		VkImage colorImage;
		VkImageView colorImageView;

		VkImage depthImage;	
		VkImageView depthImageView;
	} drawingResources = {};
    
    VulkanBufferResource stagingBufferPool;
    VulkanBufferResource staticMeshPool;
    VulkanBufferResource modelUniformBuffer;
    VulkanBufferResource sceneUniformBuffer;
   
    // Todo(Leo): Use our own arena arrays for these.
    std::vector<VulkanMesh>  			loadedMeshes;
	std::vector<VulkanTexture> 			loadedTextures;
	std::vector<VulkanMaterial>			loadedMaterials;
    std::vector<VulkanLoadedPipeline> 	loadedPipelines;
	std::vector<VulkanModel>			loadedModels;

    VulkanLoadedPipeline 	lineDrawPipeline;
    VulkanMaterial 			lineDrawMaterial;

    VulkanLoadedPipeline		guiDrawPipeline;	
	std::vector<VulkanMaterial> loadedGuiMaterials;
	VulkanMaterial 				defaultGuiMaterial;

	struct {
		VulkanTexture white;
	} debugTextures;

    // u32 currentDrawImageIndex;
    u32 currentDrawFrameIndex;
    bool32 canDraw = false;
    PipelineHandle currentBoundPipeline;
    u32 currentUniformBufferOffset;

    bool32 sceneUnloaded = false;
};

using VulkanContext = platform::Graphics;

namespace vulkan
{
	internal void
	advance_virtual_frame(VulkanContext * context)
	{
		context->virtualFrameIndex += 1;
		context->virtualFrameIndex %= VIRTUAL_FRAME_COUNT;
	}

	internal inline VulkanVirtualFrame *
	get_current_virtual_frame(VulkanContext * context)
	{
		return &context->virtualFrames[context->virtualFrameIndex];
	}

	internal inline VulkanLoadedPipeline *
	get_loaded_pipeline(VulkanContext * context, PipelineHandle handle)
	{
		return &context->loadedPipelines[handle];
	}

	internal inline VulkanTexture *
	get_loaded_texture(VulkanContext * context, TextureHandle handle)
	{
	    return &context->loadedTextures[handle];
	}

	internal inline VulkanMaterial *
	get_loaded_gui_material(VulkanContext * context, MaterialHandle handle)
	{
		return &context->loadedGuiMaterials[handle];
	}

    internal VkFormat find_supported_format(    VkPhysicalDevice physicalDevice,
                                                s32 candidateCount,
                                                VkFormat * pCandidates,
                                                VkImageTiling requestedTiling,
                                                VkFormatFeatureFlags requestedFeatures);
    internal VkFormat find_supported_depth_format(VkPhysicalDevice physicalDevice);
	internal u32 find_memory_type ( VkPhysicalDevice physicalDevice, u32 typeFilter, VkMemoryPropertyFlags properties);

	#if MAZEGAME_DEVELOPMENT
	constexpr bool32 enableValidationLayers = true;
	#else
	constexpr bool32 enableValidationLayers = false;
	#endif

	constexpr const char * validationLayers[] = {
	    "VK_LAYER_KHRONOS_validation"
	};
	constexpr int VALIDATION_LAYERS_COUNT = ARRAY_COUNT(validationLayers);

	constexpr const char * deviceExtensions [] = {
	    VK_KHR_SWAPCHAIN_EXTENSION_NAME
	};
	constexpr int DEVICE_EXTENSION_COUNT = ARRAY_COUNT(deviceExtensions);

    constexpr s32 MAX_LOADED_TEXTURES = 100;

    // Todo(Leo): this is internal to the some place
	internal VkIndexType convert_index_type(IndexType);


	internal VkVertexInputBindingDescription 					get_vertex_binding_description ();
	internal StaticArray<VkVertexInputAttributeDescription, 4> 	get_vertex_attribute_description();

	// HELPER FUNCTIONS
	internal VulkanQueueFamilyIndices find_queue_families (VkPhysicalDevice device, VkSurfaceKHR surface);
	internal VkPresentModeKHR choose_surface_present_mode(std::vector<VkPresentModeKHR> & availablePresentModes);
	internal VulkanSwapchainSupportDetails query_swap_chain_support(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface);
	internal VkSurfaceFormatKHR choose_swapchain_surface_format(std::vector<VkSurfaceFormatKHR>& availableFormats);

    internal inline bool32
    has_stencil_component(VkFormat format)
    {
        bool32 result = (format == VK_FORMAT_D32_SFLOAT_S8_UINT) || (format == VK_FORMAT_D24_UNORM_S8_UINT);
        return result;
    }

    internal inline VkDeviceSize
    get_aligned_uniform_buffer_size(VulkanContext * context, VkDeviceSize size)
    {
    	auto alignment = context->physicalDeviceProperties.limits.minUniformBufferOffsetAlignment;
    	return align_up_to(alignment, size);
    } 

    internal VkCommandBuffer begin_command_buffer(VkDevice, VkCommandPool);
    internal void execute_command_buffer(VkCommandBuffer, VkDevice, VkCommandPool, VkQueue);
	internal void cmd_transition_image_layout(	VkCommandBuffer commandBuffer,
											    VkDevice        device,
											    VkQueue         graphicsQueue,
											    VkImage         image,
											    VkFormat        format,
											    u32             mipLevels,
											    VkImageLayout   oldLayout,
											    VkImageLayout   newLayout,
											    u32             layerCount = 1);


    /// INITIALIZATION CALLS
	internal void create_drawing_resources		(VulkanContext*, u32 width, u32 height);
	internal void recreate_drawing_resources 	(VulkanContext*, u32 width, u32 height);
	internal void destroy_drawing_resources 	(VulkanContext*);					

	/// INTERNAL RESOURCES, CUSTOM
	internal VulkanLoadedPipeline make_pipeline 		(VulkanContext*, VulkanPipelineLoadInfo loadInfo);
	internal VulkanLoadedPipeline make_line_pipeline 	(VulkanContext*, VulkanPipelineLoadInfo loadInfo);
	internal VulkanLoadedPipeline make_gui_pipeline 	(VulkanContext*, VulkanPipelineLoadInfo loadInfo);
	internal void destroy_loaded_pipeline 				(VulkanContext*, VulkanLoadedPipeline * pipeline);

	internal VulkanTexture make_texture	(VulkanContext*, TextureAsset * asset);
	internal VulkanTexture make_texture	(VulkanContext * context, u32 width, u32 height, float4 color);
	internal VulkanTexture make_cubemap	(VulkanContext*, StaticArray<TextureAsset, 6> * assets);
	internal void destroy_texture 		(VulkanContext*, VulkanTexture * texture);

	internal VulkanBufferResource make_buffer_resource(	VulkanContext*,
														VkDeviceSize size,
														VkBufferUsageFlags usage,
														VkMemoryPropertyFlags memoryProperties);
	internal void destroy_buffer_resource(VkDevice device, VulkanBufferResource * resource);

	/// INTERNAL RESOURCES, VULKAN TYPES
    internal VkRenderPass 			make_vk_render_pass(VulkanContext*, VkFormat format, VkSampleCountFlagBits msaaSamples);
	internal VkDescriptorSetLayout 	make_material_vk_descriptor_set_layout(VkDevice device, u32 textureCount);
	internal VkDescriptorSet 		make_material_vk_descriptor_set(VulkanContext*, PipelineHandle pipeline, ArenaArray<TextureHandle> textures);
	internal VkDescriptorSet 		make_material_vk_descriptor_set(VulkanContext*, 
																	VulkanLoadedPipeline * pipeline,
																	VulkanTexture * texture,
																	VkDescriptorPool pool);
	internal VkFramebuffer 			make_vk_framebuffer(VkDevice, VkRenderPass, u32 attachmentCount, VkImageView * attachments, u32 width, u32 height);
	internal VkShaderModule 		make_vk_shader_module(BinaryAsset file, VkDevice logicalDevice);
	internal VkImage 				make_vk_image(	VulkanContext*, u32 width, u32 height, u32 mipLevels, VkFormat format,
											    	VkImageTiling tiling, VkImageUsageFlags usage,
											    	VkSampleCountFlagBits msaaSamples);
	internal VkImageView 			make_vk_image_view(VkDevice device, VkImage image, u32 mipLevels, VkFormat format, VkImageAspectFlags aspectFlags);

	// Note(Leo): SCENE and DRAWING functions are passed as pointers to game layer.
	/// SCENE, VulkanScene.cpp
    internal TextureHandle 	push_texture (VulkanContext*, TextureAsset * texture);
    internal TextureHandle  push_render_texture (VulkanContext*, u32 width, u32 height);
    internal TextureHandle 	push_cubemap(VulkanContext*, StaticArray<TextureAsset, 6> * assets);

    internal MaterialHandle push_material (VulkanContext*, MaterialAsset * asset);
    internal MaterialHandle push_gui_material (VulkanContext*, TextureHandle texture);
    internal MeshHandle 	push_mesh(VulkanContext*, MeshAsset * mesh);
    internal ModelHandle 	push_model (VulkanContext*, MeshHandle mesh, MaterialHandle material);
    internal PipelineHandle push_pipeline(VulkanContext*, const char * vertexShaderPath, const char * fragmentShaderPath, platform::RenderingOptions options);
    internal void 			unload_scene(VulkanContext*);

	/// DRAWING, VulkanDrawing.cpp
    internal void update_camera				(VulkanContext*, Matrix44 view, Matrix44 perspective);
	internal void prepare_drawing			(VulkanContext*);
	internal void finish_drawing 			(VulkanContext*);
	internal void record_draw_command 		(VulkanContext*, ModelHandle handle, Matrix44 transform);
	internal void record_line_draw_command	(VulkanContext*, vector3 start, vector3 end, float4 color);
	internal void record_gui_draw_command	(VulkanContext*, vector2 position, vector2 size, MaterialHandle material, float4 color);
	internal void draw_frame 				(VulkanContext * context);

	internal void prepare_shadow_pass 		(VulkanContext*, Matrix44 view, Matrix44 perspective);
	internal void finish_shadow_pass 		(VulkanContext*);
	internal void draw_shadow_model			(VulkanContext*, ModelHandle model, Matrix44 transform);

	// Lol, this is not given to game layer
}


#define WIN_VULKAN_HPP
#endif