////////////////////////////////////////////////////////////////////////////////
// Copyright 2017 Intel Corporation
//
// Licensed under the Apache License, Version 2.0 (the "License"); you may not
// use this file except in compliance with the License.  You may obtain a copy
// of the License at
//
// http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
// WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.  See the
// License for the specific language governing permissions and limitations
// under the License.
////////////////////////////////////////////////////////////////////////////////

#include "Tutorial07.h"
#include "VulkanFunctions.h"

namespace ApiWithoutSecrets {

  Tutorial07::Tutorial07() {
  }

  bool Tutorial07::CreateRenderingResources() {
    if( !CreateCommandBuffers() ) {
      return false;
    }
    if( !CreateSemaphores() ) {
      return false;
    }
    if( !CreateFences() ) {
      return false;
    }
    return true;
  }

  bool Tutorial07::CreateCommandBuffers() {
    if( !CreateCommandPool( GetGraphicsQueue().FamilyIndex, &Vulkan.CommandPool ) ) {
      std::cout << "Could not create command pool!" << std::endl;
      return false;
    }
    for( size_t i = 0; i < Vulkan.RenderingResources.size(); ++i ) {
      if( !AllocateCommandBuffers( Vulkan.CommandPool, 1, &Vulkan.RenderingResources[i].CommandBuffer ) ) {
        std::cout << "Could not allocate command buffer!" << std::endl;
        return false;
      }
    }
    return true;
  }

  bool Tutorial07::CreateCommandPool( uint32_t queue_family_index, VkCommandPool *pool ) {
    VkCommandPoolCreateInfo cmd_pool_create_info = {
      VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,       // VkStructureType                sType
      nullptr,                                          // const void                    *pNext
      VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT | // VkCommandPoolCreateFlags       flags
      VK_COMMAND_POOL_CREATE_TRANSIENT_BIT,
      queue_family_index                                // uint32_t                       queueFamilyIndex
    };

    if( vkCreateCommandPool( GetDevice(), &cmd_pool_create_info, nullptr, pool ) != VK_SUCCESS ) {
      return false;
    }
    return true;
  }

  bool Tutorial07::AllocateCommandBuffers( VkCommandPool pool, uint32_t count, VkCommandBuffer *command_buffers ) {
    VkCommandBufferAllocateInfo command_buffer_allocate_info = {
      VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,   // VkStructureType                sType
      nullptr,                                          // const void                    *pNext
      pool,                                             // VkCommandPool                  commandPool
      VK_COMMAND_BUFFER_LEVEL_PRIMARY,                  // VkCommandBufferLevel           level
      count                                             // uint32_t                       bufferCount
    };

    if( vkAllocateCommandBuffers( GetDevice(), &command_buffer_allocate_info, command_buffers ) != VK_SUCCESS ) {
      return false;
    }
    return true;
  }

  bool Tutorial07::CreateSemaphores() {
    VkSemaphoreCreateInfo semaphore_create_info = {
      VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,      // VkStructureType          sType
      nullptr,                                      // const void*              pNext
      0                                             // VkSemaphoreCreateFlags   flags
    };

    for( size_t i = 0; i < Vulkan.RenderingResources.size(); ++i ) {
      if( (vkCreateSemaphore( GetDevice(), &semaphore_create_info, nullptr, &Vulkan.RenderingResources[i].ImageAvailableSemaphore ) != VK_SUCCESS) ||
        (vkCreateSemaphore( GetDevice(), &semaphore_create_info, nullptr, &Vulkan.RenderingResources[i].FinishedRenderingSemaphore ) != VK_SUCCESS) ) {
        std::cout << "Could not create semaphores!" << std::endl;
        return false;
      }
    }
    return true;
  }

  bool Tutorial07::CreateFences() {
    VkFenceCreateInfo fence_create_info = {
      VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,              // VkStructureType                sType
      nullptr,                                          // const void                    *pNext
      VK_FENCE_CREATE_SIGNALED_BIT                      // VkFenceCreateFlags             flags
    };

    for( size_t i = 0; i < Vulkan.RenderingResources.size(); ++i ) {
      if( vkCreateFence( GetDevice(), &fence_create_info, nullptr, &Vulkan.RenderingResources[i].Fence ) != VK_SUCCESS ) {
        std::cout << "Could not create a fence!" << std::endl;
        return false;
      }
    }
    return true;
  }

  bool Tutorial07::CreateStagingBuffer() {
    Vulkan.StagingBuffer.Size = 1000000;
    if( !CreateBuffer( VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT, Vulkan.StagingBuffer ) ) {
      std::cout << "Could not create staging buffer!" << std::endl;
      return false;
    }

    return true;
  }

  bool Tutorial07::CreateBuffer( VkBufferUsageFlags usage, VkMemoryPropertyFlagBits memoryProperty, BufferParameters &buffer ) {
    VkBufferCreateInfo buffer_create_info = {
      VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,             // VkStructureType        sType
      nullptr,                                          // const void            *pNext
      0,                                                // VkBufferCreateFlags    flags
      buffer.Size,                                      // VkDeviceSize           size
      usage,                                            // VkBufferUsageFlags     usage
      VK_SHARING_MODE_EXCLUSIVE,                        // VkSharingMode          sharingMode
      0,                                                // uint32_t               queueFamilyIndexCount
      nullptr                                           // const uint32_t        *pQueueFamilyIndices
    };

    if( vkCreateBuffer( GetDevice(), &buffer_create_info, nullptr, &buffer.Handle ) != VK_SUCCESS ) {
      std::cout << "Could not create buffer!" << std::endl;
      return false;
    }

    if( !AllocateBufferMemory( buffer.Handle, memoryProperty, &buffer.Memory ) ) {
      std::cout << "Could not allocate memory for a buffer!" << std::endl;
      return false;
    }

    if( vkBindBufferMemory( GetDevice(), buffer.Handle, buffer.Memory, 0 ) != VK_SUCCESS ) {
      std::cout << "Could not bind memory to a buffer!" << std::endl;
      return false;
    }

    return true;
  }

  bool Tutorial07::AllocateBufferMemory( VkBuffer buffer, VkMemoryPropertyFlagBits property, VkDeviceMemory *memory ) {
    VkMemoryRequirements buffer_memory_requirements;
    vkGetBufferMemoryRequirements( GetDevice(), buffer, &buffer_memory_requirements );

    VkPhysicalDeviceMemoryProperties memory_properties;
    vkGetPhysicalDeviceMemoryProperties( GetPhysicalDevice(), &memory_properties );

    for( uint32_t i = 0; i < memory_properties.memoryTypeCount; ++i ) {
      if( (buffer_memory_requirements.memoryTypeBits & (1 << i)) &&
        (memory_properties.memoryTypes[i].propertyFlags & property) ) {

        VkMemoryAllocateInfo memory_allocate_info = {
          VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,     // VkStructureType                        sType
          nullptr,                                    // const void                            *pNext
          buffer_memory_requirements.size,            // VkDeviceSize                           allocationSize
          i                                           // uint32_t                               memoryTypeIndex
        };

        if( vkAllocateMemory( GetDevice(), &memory_allocate_info, nullptr, memory ) == VK_SUCCESS ) {
          return true;
        }
      }
    }
    return false;
  }

  bool Tutorial07::CreateTexture() {
    int width = 0, height = 0, data_size = 0;
    std::vector<char> texture_data = Tools::GetImageData( "../Data/Tutorials/07/texture.png", 4, &width, &height, nullptr, &data_size );
    if( texture_data.size() == 0 ) {
      return false;
    }

    if( !CreateImage( width, height, &Vulkan.Image.Handle ) ) {
      std::cout << "Could not create image!" << std::endl;
      return false;
    }

    if( !AllocateImageMemory( Vulkan.Image.Handle, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &Vulkan.Image.Memory ) ) {
      std::cout << "Could not allocate memory for image!" << std::endl;
      return false;
    }

    if( vkBindImageMemory( GetDevice(), Vulkan.Image.Handle, Vulkan.Image.Memory, 0 ) != VK_SUCCESS ) {
      std::cout << "Could not bind memory to an image!" << std::endl;
      return false;
    }

    if( !CreateImageView( Vulkan.Image ) ) {
      std::cout << "Could not create image view!" << std::endl;
      return false;
    }

    if( !CreateSampler( &Vulkan.Image.Sampler ) ) {
      std::cout << "Could not create sampler!" << std::endl;
      return false;
    }

    if( !CopyTextureData( texture_data.data(), data_size, width, height ) ) {
      std::cout << "Could not upload texture data to device memory!" << std::endl;
      return false;
    }

    return true;
  }

  bool Tutorial07::CreateImage( uint32_t width, uint32_t height, VkImage *image ) {
    VkImageCreateInfo image_create_info = {
      VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,                  // VkStructureType            sType;
      nullptr,                                              // const void                *pNext
      0,                                                    // VkImageCreateFlags         flags
      VK_IMAGE_TYPE_2D,                                     // VkImageType                imageType
      VK_FORMAT_R8G8B8A8_UNORM,                             // VkFormat                   format
      {                                                     // VkExtent3D                 extent
        width,                                                // uint32_t                   width
        height,                                               // uint32_t                   height
        1                                                     // uint32_t                   depth
      },
      1,                                                    // uint32_t                   mipLevels
      1,                                                    // uint32_t                   arrayLayers
      VK_SAMPLE_COUNT_1_BIT,                                // VkSampleCountFlagBits      samples
      VK_IMAGE_TILING_OPTIMAL,                              // VkImageTiling              tiling
      VK_IMAGE_USAGE_TRANSFER_DST_BIT |                     // VkImageUsageFlags          usage
      VK_IMAGE_USAGE_SAMPLED_BIT,
      VK_SHARING_MODE_EXCLUSIVE,                            // VkSharingMode              sharingMode
      0,                                                    // uint32_t                   queueFamilyIndexCount
      nullptr,                                              // const uint32_t*            pQueueFamilyIndices
      VK_IMAGE_LAYOUT_UNDEFINED                             // VkImageLayout              initialLayout
    };

    return vkCreateImage( GetDevice(), &image_create_info, nullptr, image ) == VK_SUCCESS;
  }

  bool Tutorial07::AllocateImageMemory( VkImage image, VkMemoryPropertyFlagBits property, VkDeviceMemory *memory ) {
    VkMemoryRequirements image_memory_requirements;
    vkGetImageMemoryRequirements( GetDevice(), image, &image_memory_requirements );

    VkPhysicalDeviceMemoryProperties memory_properties;
    vkGetPhysicalDeviceMemoryProperties( GetPhysicalDevice(), &memory_properties );

    for( uint32_t i = 0; i < memory_properties.memoryTypeCount; ++i ) {
      if( (image_memory_requirements.memoryTypeBits & (1 << i)) &&
        (memory_properties.memoryTypes[i].propertyFlags & property) ) {

        VkMemoryAllocateInfo memory_allocate_info = {
          VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,     // VkStructureType                        sType
          nullptr,                                    // const void                            *pNext
          image_memory_requirements.size,             // VkDeviceSize                           allocationSize
          i                                           // uint32_t                               memoryTypeIndex
        };

        if( vkAllocateMemory( GetDevice(), &memory_allocate_info, nullptr, memory ) == VK_SUCCESS ) {
          return true;
        }
      }
    }
    return false;
  }

  bool Tutorial07::CreateImageView( ImageParameters &image_parameters ) {
    VkImageViewCreateInfo image_view_create_info = {
      VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,             // VkStructureType            sType
      nullptr,                                              // const void                *pNext
      0,                                                    // VkImageViewCreateFlags     flags
      image_parameters.Handle,                              // VkImage                    image
      VK_IMAGE_VIEW_TYPE_2D,                                // VkImageViewType            viewType
      VK_FORMAT_R8G8B8A8_UNORM,                             // VkFormat                   format
      {                                                     // VkComponentMapping         components
        VK_COMPONENT_SWIZZLE_IDENTITY,                        // VkComponentSwizzle         r
        VK_COMPONENT_SWIZZLE_IDENTITY,                        // VkComponentSwizzle         g
        VK_COMPONENT_SWIZZLE_IDENTITY,                        // VkComponentSwizzle         b
        VK_COMPONENT_SWIZZLE_IDENTITY                         // VkComponentSwizzle         a
      },
      {                                                     // VkImageSubresourceRange    subresourceRange
        VK_IMAGE_ASPECT_COLOR_BIT,                            // VkImageAspectFlags         aspectMask
        0,                                                    // uint32_t                   baseMipLevel
        1,                                                    // uint32_t                   levelCount
        0,                                                    // uint32_t                   baseArrayLayer
        1                                                     // uint32_t                   layerCount
      }
    };

    return vkCreateImageView( GetDevice(), &image_view_create_info, nullptr, &image_parameters.View ) == VK_SUCCESS;
  }

  bool Tutorial07::CreateSampler( VkSampler *sampler ) {
    VkSamplerCreateInfo sampler_create_info = {
      VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO,                // VkStructureType            sType
      nullptr,                                              // const void*                pNext
      0,                                                    // VkSamplerCreateFlags       flags
      VK_FILTER_LINEAR,                                     // VkFilter                   magFilter
      VK_FILTER_LINEAR,                                     // VkFilter                   minFilter
      VK_SAMPLER_MIPMAP_MODE_NEAREST,                       // VkSamplerMipmapMode        mipmapMode
      VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE,                // VkSamplerAddressMode       addressModeU
      VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE,                // VkSamplerAddressMode       addressModeV
      VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE,                // VkSamplerAddressMode       addressModeW
      0.0f,                                                 // float                      mipLodBias
      VK_FALSE,                                             // VkBool32                   anisotropyEnable
      1.0f,                                                 // float                      maxAnisotropy
      VK_FALSE,                                             // VkBool32                   compareEnable
      VK_COMPARE_OP_ALWAYS,                                 // VkCompareOp                compareOp
      0.0f,                                                 // float                      minLod
      0.0f,                                                 // float                      maxLod
      VK_BORDER_COLOR_FLOAT_TRANSPARENT_BLACK,              // VkBorderColor              borderColor
      VK_FALSE                                              // VkBool32                   unnormalizedCoordinates
    };

    return vkCreateSampler( GetDevice(), &sampler_create_info, nullptr, sampler ) == VK_SUCCESS;
  }

  bool Tutorial07::CopyTextureData( char *texture_data, uint32_t data_size, uint32_t width, uint32_t height ) {
    // Prepare data in staging buffer

    void *staging_buffer_memory_pointer;
    if( vkMapMemory( GetDevice(), Vulkan.StagingBuffer.Memory, 0, data_size, 0, &staging_buffer_memory_pointer ) != VK_SUCCESS ) {
      std::cout << "Could not map memory and upload texture data to a staging buffer!" << std::endl;
      return false;
    }

    memcpy( staging_buffer_memory_pointer, texture_data, data_size );

    VkMappedMemoryRange flush_range = {
      VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE,              // VkStructureType                        sType
      nullptr,                                            // const void                            *pNext
      Vulkan.StagingBuffer.Memory,                        // VkDeviceMemory                         memory
      0,                                                  // VkDeviceSize                           offset
      data_size                                           // VkDeviceSize                           size
    };
    vkFlushMappedMemoryRanges( GetDevice(), 1, &flush_range );

    vkUnmapMemory( GetDevice(), Vulkan.StagingBuffer.Memory );

    // Prepare command buffer to copy data from staging buffer to a vertex buffer
    VkCommandBufferBeginInfo command_buffer_begin_info = {
      VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,        // VkStructureType                        sType
      nullptr,                                            // const void                            *pNext
      VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,        // VkCommandBufferUsageFlags              flags
      nullptr                                             // const VkCommandBufferInheritanceInfo  *pInheritanceInfo
    };

    VkCommandBuffer command_buffer = Vulkan.RenderingResources[0].CommandBuffer;

    vkBeginCommandBuffer( command_buffer, &command_buffer_begin_info);

    VkImageSubresourceRange image_subresource_range = {
      VK_IMAGE_ASPECT_COLOR_BIT,                          // VkImageAspectFlags                     aspectMask
      0,                                                  // uint32_t                               baseMipLevel
      1,                                                  // uint32_t                               levelCount
      0,                                                  // uint32_t                               baseArrayLayer
      1                                                   // uint32_t                               layerCount
    };

    VkImageMemoryBarrier image_memory_barrier_from_undefined_to_transfer_dst = {
      VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,             // VkStructureType                        sType
      nullptr,                                            // const void                            *pNext
      0,                                                  // VkAccessFlags                          srcAccessMask
      VK_ACCESS_TRANSFER_WRITE_BIT,                       // VkAccessFlags                          dstAccessMask
      VK_IMAGE_LAYOUT_UNDEFINED,                          // VkImageLayout                          oldLayout
      VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,               // VkImageLayout                          newLayout
      VK_QUEUE_FAMILY_IGNORED,                            // uint32_t                               srcQueueFamilyIndex
      VK_QUEUE_FAMILY_IGNORED,                            // uint32_t                               dstQueueFamilyIndex
      Vulkan.Image.Handle,                                // VkImage                                image
      image_subresource_range                             // VkImageSubresourceRange                subresourceRange
    };
    vkCmdPipelineBarrier( command_buffer, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, nullptr, 0, nullptr, 1, &image_memory_barrier_from_undefined_to_transfer_dst);

    VkBufferImageCopy buffer_image_copy_info = {
      0,                                                  // VkDeviceSize                           bufferOffset
      0,                                                  // uint32_t                               bufferRowLength
      0,                                                  // uint32_t                               bufferImageHeight
      {                                                   // VkImageSubresourceLayers               imageSubresource
        VK_IMAGE_ASPECT_COLOR_BIT,                          // VkImageAspectFlags                     aspectMask
        0,                                                  // uint32_t                               mipLevel
        0,                                                  // uint32_t                               baseArrayLayer
        1                                                   // uint32_t                               layerCount
      },
      {                                                   // VkOffset3D                             imageOffset
        0,                                                  // int32_t                                x
        0,                                                  // int32_t                                y
        0                                                   // int32_t                                z
      },
      {                                                   // VkExtent3D                             imageExtent
        width,                                              // uint32_t                               width
        height,                                             // uint32_t                               height
        1                                                   // uint32_t                               depth
      }
    };
    vkCmdCopyBufferToImage( command_buffer, Vulkan.StagingBuffer.Handle, Vulkan.Image.Handle, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &buffer_image_copy_info );

    VkImageMemoryBarrier image_memory_barrier_from_transfer_to_shader_read = {
      VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,             // VkStructureType                        sType
      nullptr,                                            // const void                            *pNext
      VK_ACCESS_TRANSFER_WRITE_BIT,                       // VkAccessFlags                          srcAccessMask
      VK_ACCESS_SHADER_READ_BIT,                          // VkAccessFlags                          dstAccessMask
      VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,               // VkImageLayout                          oldLayout
      VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,           // VkImageLayout                          newLayout
      VK_QUEUE_FAMILY_IGNORED,                            // uint32_t                               srcQueueFamilyIndex
      VK_QUEUE_FAMILY_IGNORED,                            // uint32_t                               dstQueueFamilyIndex
      Vulkan.Image.Handle,                                // VkImage                                image
      image_subresource_range                             // VkImageSubresourceRange                subresourceRange
    };
    vkCmdPipelineBarrier( command_buffer, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0, 0, nullptr, 0, nullptr, 1, &image_memory_barrier_from_transfer_to_shader_read);

    vkEndCommandBuffer( command_buffer );

    // Submit command buffer and copy data from staging buffer to a vertex buffer
    VkSubmitInfo submit_info = {
      VK_STRUCTURE_TYPE_SUBMIT_INFO,                      // VkStructureType                        sType
      nullptr,                                            // const void                            *pNext
      0,                                                  // uint32_t                               waitSemaphoreCount
      nullptr,                                            // const VkSemaphore                     *pWaitSemaphores
      nullptr,                                            // const VkPipelineStageFlags            *pWaitDstStageMask;
      1,                                                  // uint32_t                               commandBufferCount
      &command_buffer,                                    // const VkCommandBuffer                 *pCommandBuffers
      0,                                                  // uint32_t                               signalSemaphoreCount
      nullptr                                             // const VkSemaphore                     *pSignalSemaphores
    };

    if( vkQueueSubmit( GetGraphicsQueue().Handle, 1, &submit_info, VK_NULL_HANDLE ) != VK_SUCCESS ) {
      return false;
    }

    vkDeviceWaitIdle( GetDevice() );

    return true;
  }

  bool Tutorial07::CreateUniformBuffer() {
    Vulkan.UniformBuffer.Size = 16 * sizeof(float);
    if( !CreateBuffer( VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, Vulkan.UniformBuffer ) ) {
      std::cout << "Could not create uniform buffer!" << std::endl;
      return false;
    }

    if( !CopyUniformBufferData() ) {
      return false;
    }

    return true;
  }

  const std::array<float, 16> Tutorial07::GetUniformBufferData() const {
    float half_width = static_cast<float>(GetSwapChain().Extent.width) * 0.5f;
    float half_height = static_cast<float>(GetSwapChain().Extent.height) * 0.5f;

    return Tools::GetOrthographicProjectionMatrix( -half_width, half_width, -half_height, half_height, -1.0f, 1.0f );
  }

  bool Tutorial07::CopyUniformBufferData() {
    // Prepare data in staging buffer
    const std::array<float, 16> uniform_data = GetUniformBufferData();

    void *staging_buffer_memory_pointer;
    if( vkMapMemory( GetDevice(), Vulkan.StagingBuffer.Memory, 0, Vulkan.UniformBuffer.Size, 0, &staging_buffer_memory_pointer) != VK_SUCCESS ) {
      std::cout << "Could not map memory and upload data to a staging buffer!" << std::endl;
      return false;
    }

    memcpy( staging_buffer_memory_pointer, uniform_data.data(), Vulkan.UniformBuffer.Size );

    VkMappedMemoryRange flush_range = {
      VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE,  // VkStructureType  sType
      nullptr,                                // const void      *pNext
      Vulkan.StagingBuffer.Memory,            // VkDeviceMemory   memory
      0,                                      // VkDeviceSize     offset
      Vulkan.UniformBuffer.Size               // VkDeviceSize     size
    };
    vkFlushMappedMemoryRanges( GetDevice(), 1, &flush_range );

    vkUnmapMemory( GetDevice(), Vulkan.StagingBuffer.Memory );

    // Prepare command buffer to copy data from staging buffer to a uniform buffer
    VkCommandBuffer command_buffer = Vulkan.RenderingResources[0].CommandBuffer;

    VkCommandBufferBeginInfo command_buffer_begin_info = {
      VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,        // VkStructureType                        sType
      nullptr,                                            // const void                            *pNext
      VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,        // VkCommandBufferUsageFlags              flags
      nullptr                                             // const VkCommandBufferInheritanceInfo  *pInheritanceInfo
    };

    vkBeginCommandBuffer( command_buffer, &command_buffer_begin_info);

    VkBufferCopy buffer_copy_info = {
      0,                                                  // VkDeviceSize                           srcOffset
      0,                                                  // VkDeviceSize                           dstOffset
      Vulkan.UniformBuffer.Size                           // VkDeviceSize                           size
    };
    vkCmdCopyBuffer( command_buffer, Vulkan.StagingBuffer.Handle, Vulkan.UniformBuffer.Handle, 1, &buffer_copy_info );

    VkBufferMemoryBarrier buffer_memory_barrier = {
      VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER,            // VkStructureType                        sType;
      nullptr,                                            // const void                            *pNext
      VK_ACCESS_TRANSFER_WRITE_BIT,                       // VkAccessFlags                          srcAccessMask
      VK_ACCESS_UNIFORM_READ_BIT,                         // VkAccessFlags                          dstAccessMask
      VK_QUEUE_FAMILY_IGNORED,                            // uint32_t                               srcQueueFamilyIndex
      VK_QUEUE_FAMILY_IGNORED,                            // uint32_t                               dstQueueFamilyIndex
      Vulkan.UniformBuffer.Handle,                        // VkBuffer                               buffer
      0,                                                  // VkDeviceSize                           offset
      VK_WHOLE_SIZE                                       // VkDeviceSize                           size
    };
    vkCmdPipelineBarrier( command_buffer, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_VERTEX_SHADER_BIT, 0, 0, nullptr, 1, &buffer_memory_barrier, 0, nullptr );

    vkEndCommandBuffer( command_buffer );

    // Submit command buffer and copy data from staging buffer to a vertex buffer
    VkSubmitInfo submit_info = {
      VK_STRUCTURE_TYPE_SUBMIT_INFO,                      // VkStructureType                        sType
      nullptr,                                            // const void                            *pNext
      0,                                                  // uint32_t                               waitSemaphoreCount
      nullptr,                                            // const VkSemaphore                     *pWaitSemaphores
      nullptr,                                            // const VkPipelineStageFlags            *pWaitDstStageMask;
      1,                                                  // uint32_t                               commandBufferCount
      &command_buffer,                                    // const VkCommandBuffer                 *pCommandBuffers
      0,                                                  // uint32_t                               signalSemaphoreCount
      nullptr                                             // const VkSemaphore                     *pSignalSemaphores
    };

    if( vkQueueSubmit( GetGraphicsQueue().Handle, 1, &submit_info, VK_NULL_HANDLE ) != VK_SUCCESS ) {
      return false;
    }

    vkDeviceWaitIdle( GetDevice() );
    return true;
  }

  bool Tutorial07::CreateDescriptorSetLayout() {
    std::vector<VkDescriptorSetLayoutBinding> layout_bindings = {
      {
        0,                                                  // uint32_t                             binding
        VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,          // VkDescriptorType                     descriptorType
        1,                                                  // uint32_t                             descriptorCount
        VK_SHADER_STAGE_FRAGMENT_BIT,                       // VkShaderStageFlags                   stageFlags
        nullptr                                             // const VkSampler                     *pImmutableSamplers
      },
      {
        1,                                                  // uint32_t                             binding
        VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,                  // VkDescriptorType                     descriptorType
        1,                                                  // uint32_t                             descriptorCount
        VK_SHADER_STAGE_VERTEX_BIT,                         // VkShaderStageFlags                   stageFlags
        nullptr                                             // const VkSampler                     *pImmutableSamplers
      }
    };

    VkDescriptorSetLayoutCreateInfo descriptor_set_layout_create_info = {
      VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,  // VkStructureType                      sType
      nullptr,                                              // const void                          *pNext
      0,                                                    // VkDescriptorSetLayoutCreateFlags     flags
      static_cast<uint32_t>(layout_bindings.size()),        // uint32_t                             bindingCount
      layout_bindings.data()                                // const VkDescriptorSetLayoutBinding  *pBindings
    };

    if( vkCreateDescriptorSetLayout( GetDevice(), &descriptor_set_layout_create_info, nullptr, &Vulkan.DescriptorSet.Layout ) != VK_SUCCESS ) {
      std::cout << "Could not create descriptor set layout!" << std::endl;
      return false;
    }

    return true;
  }

  bool Tutorial07::CreateDescriptorPool() {
    std::vector<VkDescriptorPoolSize> pool_sizes = {
      {
        VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,          // VkDescriptorType                     type
        1                                                   // uint32_t                             descriptorCount
      },
      {
        VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,                  // VkDescriptorType                     type
        1                                                   // uint32_t                             descriptorCount
      }
    };

    VkDescriptorPoolCreateInfo descriptor_pool_create_info = {
      VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,        // VkStructureType                      sType
      nullptr,                                              // const void                          *pNext
      0,                                                    // VkDescriptorPoolCreateFlags          flags
      1,                                                    // uint32_t                             maxSets
      static_cast<uint32_t>(pool_sizes.size()),             // uint32_t                             poolSizeCount
      pool_sizes.data()                                     // const VkDescriptorPoolSize          *pPoolSizes
    };

    if( vkCreateDescriptorPool( GetDevice(), &descriptor_pool_create_info, nullptr, &Vulkan.DescriptorSet.Pool ) != VK_SUCCESS ) {
      std::cout << "Could not create descriptor pool!" << std::endl;
      return false;
    }

    return true;
  }

  bool Tutorial07::AllocateDescriptorSet() {
    VkDescriptorSetAllocateInfo descriptor_set_allocate_info = {
      VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO, // VkStructureType                sType
      nullptr,                                        // const void                    *pNext
      Vulkan.DescriptorSet.Pool,                      // VkDescriptorPool               descriptorPool
      1,                                              // uint32_t                       descriptorSetCount
      &Vulkan.DescriptorSet.Layout                    // const VkDescriptorSetLayout   *pSetLayouts
    };

    if( vkAllocateDescriptorSets( GetDevice(), &descriptor_set_allocate_info, &Vulkan.DescriptorSet.Handle ) != VK_SUCCESS ) {
      std::cout << "Could not allocate descriptor set!" << std::endl;
      return false;
    }

    return true;
  }

  bool Tutorial07::UpdateDescriptorSet() {
    VkDescriptorImageInfo image_info = {
      Vulkan.Image.Sampler,                           // VkSampler                      sampler
      Vulkan.Image.View,                              // VkImageView                    imageView
      VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL        // VkImageLayout                  imageLayout
    };

    VkDescriptorBufferInfo buffer_info = {
      Vulkan.UniformBuffer.Handle,                    // VkBuffer                       buffer
      0,                                              // VkDeviceSize                   offset
      Vulkan.UniformBuffer.Size                       // VkDeviceSize                   range
    };

    std::vector<VkWriteDescriptorSet> descriptor_writes = {
      {
        VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,       // VkStructureType                sType
        nullptr,                                      // const void                    *pNext
        Vulkan.DescriptorSet.Handle,                  // VkDescriptorSet                dstSet
        0,                                            // uint32_t                       dstBinding
        0,                                            // uint32_t                       dstArrayElement
        1,                                            // uint32_t                       descriptorCount
        VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,    // VkDescriptorType               descriptorType
        &image_info,                                  // const VkDescriptorImageInfo   *pImageInfo
        nullptr,                                      // const VkDescriptorBufferInfo  *pBufferInfo
        nullptr                                       // const VkBufferView            *pTexelBufferView
      },
      {
        VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,       // VkStructureType                sType
        nullptr,                                      // const void                    *pNext
        Vulkan.DescriptorSet.Handle,                  // VkDescriptorSet                dstSet
        1,                                            // uint32_t                       dstBinding
        0,                                            // uint32_t                       dstArrayElement
        1,                                            // uint32_t                       descriptorCount
        VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,            // VkDescriptorType               descriptorType
        nullptr,                                      // const VkDescriptorImageInfo   *pImageInfo
        &buffer_info,                                 // const VkDescriptorBufferInfo  *pBufferInfo
        nullptr                                       // const VkBufferView            *pTexelBufferView
      }
    };

    vkUpdateDescriptorSets( GetDevice(), static_cast<uint32_t>(descriptor_writes.size()), descriptor_writes.data(), 0, nullptr );
    return true;
  }

  bool Tutorial07::CreateRenderPass() {
    VkAttachmentDescription attachment_descriptions[] = {
      {
        0,                                          // VkAttachmentDescriptionFlags   flags
        GetSwapChain().Format,                      // VkFormat                       format
        VK_SAMPLE_COUNT_1_BIT,                      // VkSampleCountFlagBits          samples
        VK_ATTACHMENT_LOAD_OP_CLEAR,                // VkAttachmentLoadOp             loadOp
        VK_ATTACHMENT_STORE_OP_STORE,               // VkAttachmentStoreOp            storeOp
        VK_ATTACHMENT_LOAD_OP_DONT_CARE,            // VkAttachmentLoadOp             stencilLoadOp
        VK_ATTACHMENT_STORE_OP_DONT_CARE,           // VkAttachmentStoreOp            stencilStoreOp
        VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,   // VkImageLayout                  initialLayout;
        VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL    // VkImageLayout                  finalLayout
      }
    };

    VkAttachmentReference color_attachment_references[] = {
      {
        0,                                          // uint32_t                       attachment
        VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL    // VkImageLayout                  layout
      }
    };

    VkSubpassDescription subpass_descriptions[] = {
      {
        0,                                          // VkSubpassDescriptionFlags      flags
        VK_PIPELINE_BIND_POINT_GRAPHICS,            // VkPipelineBindPoint            pipelineBindPoint
        0,                                          // uint32_t                       inputAttachmentCount
        nullptr,                                    // const VkAttachmentReference   *pInputAttachments
        1,                                          // uint32_t                       colorAttachmentCount
        color_attachment_references,                // const VkAttachmentReference   *pColorAttachments
        nullptr,                                    // const VkAttachmentReference   *pResolveAttachments
        nullptr,                                    // const VkAttachmentReference   *pDepthStencilAttachment
        0,                                          // uint32_t                       preserveAttachmentCount
        nullptr                                     // const uint32_t*                pPreserveAttachments
      }
    };

    VkRenderPassCreateInfo render_pass_create_info = {
      VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,    // VkStructureType                sType
      nullptr,                                      // const void                    *pNext
      0,                                            // VkRenderPassCreateFlags        flags
      1,                                            // uint32_t                       attachmentCount
      attachment_descriptions,                      // const VkAttachmentDescription *pAttachments
      1,                                            // uint32_t                       subpassCount
      subpass_descriptions,                         // const VkSubpassDescription    *pSubpasses
      0,                                            // uint32_t                       dependencyCount
      nullptr                                       // const VkSubpassDependency     *pDependencies
    };

    if( vkCreateRenderPass( GetDevice(), &render_pass_create_info, nullptr, &Vulkan.RenderPass ) != VK_SUCCESS ) {
      std::cout << "Could not create render pass!" << std::endl;
      return false;
    }

    return true;
  }

  bool Tutorial07::CreatePipelineLayout() {
    VkPipelineLayoutCreateInfo layout_create_info = {
      VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,  // VkStructureType                sType
      nullptr,                                        // const void                    *pNext
      0,                                              // VkPipelineLayoutCreateFlags    flags
      1,                                              // uint32_t                       setLayoutCount
      &Vulkan.DescriptorSet.Layout,                   // const VkDescriptorSetLayout   *pSetLayouts
      0,                                              // uint32_t                       pushConstantRangeCount
      nullptr                                         // const VkPushConstantRange     *pPushConstantRanges
    };

    if( vkCreatePipelineLayout( GetDevice(), &layout_create_info, nullptr, &Vulkan.PipelineLayout ) != VK_SUCCESS ) {
      std::cout << "Could not create pipeline layout!" << std::endl;
      return false;
    }
    return true;
  }

  bool Tutorial07::CreatePipeline() {
    Tools::AutoDeleter<VkShaderModule, PFN_vkDestroyShaderModule> vertex_shader_module = CreateShaderModule( "../Data/Tutorials/07/shader.vert.spv" );
    Tools::AutoDeleter<VkShaderModule, PFN_vkDestroyShaderModule> fragment_shader_module = CreateShaderModule( "../Data/Tutorials/07/shader.frag.spv" );

    if( !vertex_shader_module || !fragment_shader_module ) {
      return false;
    }

    std::vector<VkPipelineShaderStageCreateInfo> shader_stage_create_infos = {
      // Vertex shader
      {
        VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,        // VkStructureType                                sType
        nullptr,                                                    // const void                                    *pNext
        0,                                                          // VkPipelineShaderStageCreateFlags               flags
        VK_SHADER_STAGE_VERTEX_BIT,                                 // VkShaderStageFlagBits                          stage
        vertex_shader_module.Get(),                                 // VkShaderModule                                 module
        "main",                                                     // const char                                    *pName
        nullptr                                                     // const VkSpecializationInfo                    *pSpecializationInfo
      },
      // Fragment shader
      {
        VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,        // VkStructureType                                sType
        nullptr,                                                    // const void                                    *pNext
        0,                                                          // VkPipelineShaderStageCreateFlags               flags
        VK_SHADER_STAGE_FRAGMENT_BIT,                               // VkShaderStageFlagBits                          stage
        fragment_shader_module.Get(),                               // VkShaderModule                                 module
        "main",                                                     // const char                                    *pName
        nullptr                                                     // const VkSpecializationInfo                    *pSpecializationInfo
      }
    };

    VkVertexInputBindingDescription vertex_binding_description = {
      0,                                                            // uint32_t                                       binding
      sizeof(VertexData),                                           // uint32_t                                       stride
      VK_VERTEX_INPUT_RATE_VERTEX                                   // VkVertexInputRate                              inputRate
    };

    VkVertexInputAttributeDescription vertex_attribute_descriptions[] = {
      {
        0,                                                          // uint32_t                                       location
        vertex_binding_description.binding,                         // uint32_t                                       binding
        VK_FORMAT_R32G32B32A32_SFLOAT,                              // VkFormat                                       format
        0                                                           // uint32_t                                       offset
      },
      {
        1,                                                          // uint32_t                                       location
        vertex_binding_description.binding,                         // uint32_t                                       binding
        VK_FORMAT_R32G32_SFLOAT,                                    // VkFormat                                       format
        4 * sizeof(float)                                           // uint32_t                                       offset
      }
    };

    VkPipelineVertexInputStateCreateInfo vertex_input_state_create_info = {
      VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,    // VkStructureType                                sType
      nullptr,                                                      // const void                                    *pNext
      0,                                                            // VkPipelineVertexInputStateCreateFlags          flags;
      1,                                                            // uint32_t                                       vertexBindingDescriptionCount
      &vertex_binding_description,                                  // const VkVertexInputBindingDescription         *pVertexBindingDescriptions
      2,                                                            // uint32_t                                       vertexAttributeDescriptionCount
      vertex_attribute_descriptions                                 // const VkVertexInputAttributeDescription       *pVertexAttributeDescriptions
    };

    VkPipelineInputAssemblyStateCreateInfo input_assembly_state_create_info = {
      VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,  // VkStructureType                                sType
      nullptr,                                                      // const void                                    *pNext
      0,                                                            // VkPipelineInputAssemblyStateCreateFlags        flags
      VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP,                         // VkPrimitiveTopology                            topology
      VK_FALSE                                                      // VkBool32                                       primitiveRestartEnable
    };

    VkPipelineViewportStateCreateInfo viewport_state_create_info = {
      VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,        // VkStructureType                                sType
      nullptr,                                                      // const void                                    *pNext
      0,                                                            // VkPipelineViewportStateCreateFlags             flags
      1,                                                            // uint32_t                                       viewportCount
      nullptr,                                                      // const VkViewport                              *pViewports
      1,                                                            // uint32_t                                       scissorCount
      nullptr                                                       // const VkRect2D                                *pScissors
    };

    VkPipelineRasterizationStateCreateInfo rasterization_state_create_info = {
      VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,   // VkStructureType                                sType
      nullptr,                                                      // const void                                    *pNext
      0,                                                            // VkPipelineRasterizationStateCreateFlags        flags
      VK_FALSE,                                                     // VkBool32                                       depthClampEnable
      VK_FALSE,                                                     // VkBool32                                       rasterizerDiscardEnable
      VK_POLYGON_MODE_FILL,                                         // VkPolygonMode                                  polygonMode
      VK_CULL_MODE_BACK_BIT,                                        // VkCullModeFlags                                cullMode
      VK_FRONT_FACE_COUNTER_CLOCKWISE,                              // VkFrontFace                                    frontFace
      VK_FALSE,                                                     // VkBool32                                       depthBiasEnable
      0.0f,                                                         // float                                          depthBiasConstantFactor
      0.0f,                                                         // float                                          depthBiasClamp
      0.0f,                                                         // float                                          depthBiasSlopeFactor
      1.0f                                                          // float                                          lineWidth
    };

    VkPipelineMultisampleStateCreateInfo multisample_state_create_info = {
      VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,     // VkStructureType                                sType
      nullptr,                                                      // const void                                    *pNext
      0,                                                            // VkPipelineMultisampleStateCreateFlags          flags
      VK_SAMPLE_COUNT_1_BIT,                                        // VkSampleCountFlagBits                          rasterizationSamples
      VK_FALSE,                                                     // VkBool32                                       sampleShadingEnable
      1.0f,                                                         // float                                          minSampleShading
      nullptr,                                                      // const VkSampleMask                            *pSampleMask
      VK_FALSE,                                                     // VkBool32                                       alphaToCoverageEnable
      VK_FALSE                                                      // VkBool32                                       alphaToOneEnable
    };

    VkPipelineColorBlendAttachmentState color_blend_attachment_state = {
      VK_FALSE,                                                     // VkBool32                                       blendEnable
      VK_BLEND_FACTOR_ONE,                                          // VkBlendFactor                                  srcColorBlendFactor
      VK_BLEND_FACTOR_ZERO,                                         // VkBlendFactor                                  dstColorBlendFactor
      VK_BLEND_OP_ADD,                                              // VkBlendOp                                      colorBlendOp
      VK_BLEND_FACTOR_ONE,                                          // VkBlendFactor                                  srcAlphaBlendFactor
      VK_BLEND_FACTOR_ZERO,                                         // VkBlendFactor                                  dstAlphaBlendFactor
      VK_BLEND_OP_ADD,                                              // VkBlendOp                                      alphaBlendOp
      VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |         // VkColorComponentFlags                          colorWriteMask
      VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT
    };

    VkPipelineColorBlendStateCreateInfo color_blend_state_create_info = {
      VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,     // VkStructureType                                sType
      nullptr,                                                      // const void                                    *pNext
      0,                                                            // VkPipelineColorBlendStateCreateFlags           flags
      VK_FALSE,                                                     // VkBool32                                       logicOpEnable
      VK_LOGIC_OP_COPY,                                             // VkLogicOp                                      logicOp
      1,                                                            // uint32_t                                       attachmentCount
      &color_blend_attachment_state,                                // const VkPipelineColorBlendAttachmentState     *pAttachments
      { 0.0f, 0.0f, 0.0f, 0.0f }                                    // float                                          blendConstants[4]
    };

    VkDynamicState dynamic_states[] = {
      VK_DYNAMIC_STATE_VIEWPORT,
      VK_DYNAMIC_STATE_SCISSOR,
    };

    VkPipelineDynamicStateCreateInfo dynamic_state_create_info = {
      VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO,         // VkStructureType                                sType
      nullptr,                                                      // const void                                    *pNext
      0,                                                            // VkPipelineDynamicStateCreateFlags              flags
      2,                                                            // uint32_t                                       dynamicStateCount
      dynamic_states                                                // const VkDynamicState                          *pDynamicStates
    };

    VkGraphicsPipelineCreateInfo pipeline_create_info = {
      VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,              // VkStructureType                                sType
      nullptr,                                                      // const void                                    *pNext
      0,                                                            // VkPipelineCreateFlags                          flags
      static_cast<uint32_t>(shader_stage_create_infos.size()),      // uint32_t                                       stageCount
      shader_stage_create_infos.data(),                             // const VkPipelineShaderStageCreateInfo         *pStages
      &vertex_input_state_create_info,                              // const VkPipelineVertexInputStateCreateInfo    *pVertexInputState;
      &input_assembly_state_create_info,                            // const VkPipelineInputAssemblyStateCreateInfo  *pInputAssemblyState
      nullptr,                                                      // const VkPipelineTessellationStateCreateInfo   *pTessellationState
      &viewport_state_create_info,                                  // const VkPipelineViewportStateCreateInfo       *pViewportState
      &rasterization_state_create_info,                             // const VkPipelineRasterizationStateCreateInfo  *pRasterizationState
      &multisample_state_create_info,                               // const VkPipelineMultisampleStateCreateInfo    *pMultisampleState
      nullptr,                                                      // const VkPipelineDepthStencilStateCreateInfo   *pDepthStencilState
      &color_blend_state_create_info,                               // const VkPipelineColorBlendStateCreateInfo     *pColorBlendState
      &dynamic_state_create_info,                                   // const VkPipelineDynamicStateCreateInfo        *pDynamicState
      Vulkan.PipelineLayout,                                        // VkPipelineLayout                               layout
      Vulkan.RenderPass,                                            // VkRenderPass                                   renderPass
      0,                                                            // uint32_t                                       subpass
      VK_NULL_HANDLE,                                               // VkPipeline                                     basePipelineHandle
      -1                                                            // int32_t                                        basePipelineIndex
    };

    if( vkCreateGraphicsPipelines( GetDevice(), VK_NULL_HANDLE, 1, &pipeline_create_info, nullptr, &Vulkan.GraphicsPipeline ) != VK_SUCCESS ) {
      std::cout << "Could not create graphics pipeline!" << std::endl;
      return false;
    }
    return true;
  }

  Tools::AutoDeleter<VkShaderModule, PFN_vkDestroyShaderModule> Tutorial07::CreateShaderModule( const char* filename ) {
    const std::vector<char> code = Tools::GetBinaryFileContents( filename );
    if( code.size() == 0 ) {
      return Tools::AutoDeleter<VkShaderModule, PFN_vkDestroyShaderModule>();
    }

    VkShaderModuleCreateInfo shader_module_create_info = {
      VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,    // VkStructureType                sType
      nullptr,                                        // const void                    *pNext
      0,                                              // VkShaderModuleCreateFlags      flags
      code.size(),                                    // size_t                         codeSize
      reinterpret_cast<const uint32_t*>(code.data())  // const uint32_t                *pCode
    };

    VkShaderModule shader_module;
    if( vkCreateShaderModule( GetDevice(), &shader_module_create_info, nullptr, &shader_module ) != VK_SUCCESS ) {
      std::cout << "Could not create shader module from a \"" << filename << "\" file!" << std::endl;
      return Tools::AutoDeleter<VkShaderModule, PFN_vkDestroyShaderModule>();
    }

    return Tools::AutoDeleter<VkShaderModule, PFN_vkDestroyShaderModule>( shader_module, vkDestroyShaderModule, GetDevice() );
  }

  bool Tutorial07::CreateVertexBuffer() {
    const std::vector<float>& vertex_data = GetVertexData();

    Vulkan.VertexBuffer.Size = static_cast<uint32_t>(vertex_data.size() * sizeof(vertex_data[0]));
    if( !CreateBuffer( VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, Vulkan.VertexBuffer ) ) {
      std::cout << "Could not create vertex buffer!" << std::endl;
      return false;
    }

    if( !CopyVertexData() ) {
      return false;
    }

    return true;
  }

  const std::vector<float>& Tutorial07::GetVertexData() const {
    static const std::vector<float> vertex_data = {
      -170.0f, -170.0f, 0.0f, 1.0f,
      -0.1f, -0.1f,
      //
      -170.0f, 170.0f, 0.0f, 1.0f,
      -0.1f, 1.1f,
      //
      170.0f, -170.0f, 0.0f, 1.0f,
      1.1f, -0.1f,
      //
      170.0f, 170.0f, 0.0f, 1.0f,
      1.1f, 1.1f,
    };

    return vertex_data;
  }

  bool Tutorial07::CopyVertexData() {
    // Prepare data in staging buffer
    const std::vector<float>& vertex_data = GetVertexData();

    void *staging_buffer_memory_pointer;
    if( vkMapMemory( GetDevice(), Vulkan.StagingBuffer.Memory, 0, Vulkan.VertexBuffer.Size, 0, &staging_buffer_memory_pointer) != VK_SUCCESS ) {
      std::cout << "Could not map memory and upload data to a staging buffer!" << std::endl;
      return false;
    }

    memcpy( staging_buffer_memory_pointer, vertex_data.data(), Vulkan.VertexBuffer.Size );

    VkMappedMemoryRange flush_range = {
      VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE,              // VkStructureType                        sType
      nullptr,                                            // const void                            *pNext
      Vulkan.StagingBuffer.Memory,                        // VkDeviceMemory                         memory
      0,                                                  // VkDeviceSize                           offset
      Vulkan.VertexBuffer.Size                            // VkDeviceSize                           size
    };
    vkFlushMappedMemoryRanges( GetDevice(), 1, &flush_range );

    vkUnmapMemory( GetDevice(), Vulkan.StagingBuffer.Memory );

    // Prepare command buffer to copy data from staging buffer to a vertex buffer
    VkCommandBufferBeginInfo command_buffer_begin_info = {
      VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,        // VkStructureType                        sType
      nullptr,                                            // const void                            *pNext
      VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,        // VkCommandBufferUsageFlags              flags
      nullptr                                             // const VkCommandBufferInheritanceInfo  *pInheritanceInfo
    };

    VkCommandBuffer command_buffer = Vulkan.RenderingResources[0].CommandBuffer;

    vkBeginCommandBuffer( command_buffer, &command_buffer_begin_info);

    VkBufferCopy buffer_copy_info = {
      0,                                                  // VkDeviceSize                           srcOffset
      0,                                                  // VkDeviceSize                           dstOffset
      Vulkan.VertexBuffer.Size                            // VkDeviceSize                           size
    };
    vkCmdCopyBuffer(command_buffer, Vulkan.StagingBuffer.Handle, Vulkan.VertexBuffer.Handle, 1, &buffer_copy_info);

    VkBufferMemoryBarrier buffer_memory_barrier = {
      VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER,            // VkStructureType                        sType;
      nullptr,                                            // const void                            *pNext
      VK_ACCESS_MEMORY_WRITE_BIT,                         // VkAccessFlags                          srcAccessMask
      VK_ACCESS_VERTEX_ATTRIBUTE_READ_BIT,                // VkAccessFlags                          dstAccessMask
      VK_QUEUE_FAMILY_IGNORED,                            // uint32_t                               srcQueueFamilyIndex
      VK_QUEUE_FAMILY_IGNORED,                            // uint32_t                               dstQueueFamilyIndex
      Vulkan.VertexBuffer.Handle,                         // VkBuffer                               buffer
      0,                                                  // VkDeviceSize                           offset
      VK_WHOLE_SIZE                                       // VkDeviceSize                           size
    };
    vkCmdPipelineBarrier( command_buffer, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_VERTEX_INPUT_BIT, 0, 0, nullptr, 1, &buffer_memory_barrier, 0, nullptr );

    vkEndCommandBuffer( command_buffer );

    // Submit command buffer and copy data from staging buffer to a vertex buffer
    VkSubmitInfo submit_info = {
      VK_STRUCTURE_TYPE_SUBMIT_INFO,                      // VkStructureType                        sType
      nullptr,                                            // const void                            *pNext
      0,                                                  // uint32_t                               waitSemaphoreCount
      nullptr,                                            // const VkSemaphore                     *pWaitSemaphores
      nullptr,                                            // const VkPipelineStageFlags            *pWaitDstStageMask;
      1,                                                  // uint32_t                               commandBufferCount
      &command_buffer,                                    // const VkCommandBuffer                 *pCommandBuffers
      0,                                                  // uint32_t                               signalSemaphoreCount
      nullptr                                             // const VkSemaphore                     *pSignalSemaphores
    };

    if( vkQueueSubmit( GetGraphicsQueue().Handle, 1, &submit_info, VK_NULL_HANDLE ) != VK_SUCCESS ) {
      return false;
    }

    vkDeviceWaitIdle( GetDevice() );

    return true;
  }

  bool Tutorial07::PrepareFrame( VkCommandBuffer command_buffer, const ImageParameters &image_parameters, VkFramebuffer &framebuffer ) {
    if( !CreateFramebuffer( framebuffer, image_parameters.View ) ) {
      return false;
    }

    VkCommandBufferBeginInfo command_buffer_begin_info = {
      VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,        // VkStructureType                        sType
      nullptr,                                            // const void                            *pNext
      VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,        // VkCommandBufferUsageFlags              flags
      nullptr                                             // const VkCommandBufferInheritanceInfo  *pInheritanceInfo
    };

    vkBeginCommandBuffer( command_buffer, &command_buffer_begin_info );

    VkImageSubresourceRange image_subresource_range = {
      VK_IMAGE_ASPECT_COLOR_BIT,                          // VkImageAspectFlags                     aspectMask
      0,                                                  // uint32_t                               baseMipLevel
      1,                                                  // uint32_t                               levelCount
      0,                                                  // uint32_t                               baseArrayLayer
      1                                                   // uint32_t                               layerCount
    };

    uint32_t present_queue_family_index = (GetPresentQueue().Handle != GetGraphicsQueue().Handle) ? GetPresentQueue().FamilyIndex : VK_QUEUE_FAMILY_IGNORED;
    uint32_t graphics_queue_family_index = (GetPresentQueue().Handle != GetGraphicsQueue().Handle) ? GetGraphicsQueue().FamilyIndex : VK_QUEUE_FAMILY_IGNORED;
    VkImageMemoryBarrier barrier_from_present_to_draw = {
      VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,             // VkStructureType                        sType
      nullptr,                                            // const void                            *pNext
      VK_ACCESS_MEMORY_READ_BIT,                          // VkAccessFlags                          srcAccessMask
      VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,               // VkAccessFlags                          dstAccessMask
      VK_IMAGE_LAYOUT_UNDEFINED,                          // VkImageLayout                          oldLayout
      VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,           // VkImageLayout                          newLayout
      present_queue_family_index,                         // uint32_t                               srcQueueFamilyIndex
      graphics_queue_family_index,                        // uint32_t                               dstQueueFamilyIndex
      image_parameters.Handle,                            // VkImage                                image
      image_subresource_range                             // VkImageSubresourceRange                subresourceRange
    };
    vkCmdPipelineBarrier( command_buffer, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, 0, 0, nullptr, 0, nullptr, 1, &barrier_from_present_to_draw );

    VkClearValue clear_value = {
      { 1.0f, 0.8f, 0.4f, 0.0f },                         // VkClearColorValue                      color
    };

    VkRenderPassBeginInfo render_pass_begin_info = {
      VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,           // VkStructureType                        sType
      nullptr,                                            // const void                            *pNext
      Vulkan.RenderPass,                                  // VkRenderPass                           renderPass
      framebuffer,                                        // VkFramebuffer                          framebuffer
      {                                                   // VkRect2D                               renderArea
        {                                                 // VkOffset2D                             offset
          0,                                                // int32_t                                x
          0                                                 // int32_t                                y
        },
        GetSwapChain().Extent,                            // VkExtent2D                             extent;
      },
      1,                                                  // uint32_t                               clearValueCount
      &clear_value                                        // const VkClearValue                    *pClearValues
    };

    vkCmdBeginRenderPass( command_buffer, &render_pass_begin_info, VK_SUBPASS_CONTENTS_INLINE );

    vkCmdBindPipeline( command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, Vulkan.GraphicsPipeline );

    VkViewport viewport = {
      0.0f,                                               // float                                  x
      0.0f,                                               // float                                  y
      static_cast<float>(GetSwapChain().Extent.width),    // float                                  width
      static_cast<float>(GetSwapChain().Extent.height),   // float                                  height
      0.0f,                                               // float                                  minDepth
      1.0f                                                // float                                  maxDepth
    };

    VkRect2D scissor = {
      {                                                   // VkOffset2D                             offset
        0,                                                  // int32_t                                x
        0                                                   // int32_t                                y
      },
      {                                                   // VkExtent2D                             extent
        GetSwapChain().Extent.width,                        // uint32_t                               width
        GetSwapChain().Extent.height                        // uint32_t                               height
      }
    };

    vkCmdSetViewport( command_buffer, 0, 1, &viewport );
    vkCmdSetScissor( command_buffer, 0, 1, &scissor );

    VkDeviceSize offset = 0;
    vkCmdBindVertexBuffers( command_buffer, 0, 1, &Vulkan.VertexBuffer.Handle, &offset );

    vkCmdBindDescriptorSets( command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, Vulkan.PipelineLayout, 0, 1, &Vulkan.DescriptorSet.Handle, 0, nullptr );

    vkCmdDraw( command_buffer, 4, 1, 0, 0 );

    vkCmdEndRenderPass( command_buffer );

    VkImageMemoryBarrier barrier_from_draw_to_present = {
      VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,             // VkStructureType                        sType
      nullptr,                                            // const void                            *pNext
      VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,               // VkAccessFlags                          srcAccessMask
      VK_ACCESS_MEMORY_READ_BIT,                          // VkAccessFlags                          dstAccessMask
      VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,           // VkImageLayout                          oldLayout
      VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,                    // VkImageLayout                          newLayout
      graphics_queue_family_index,                        // uint32_t                               srcQueueFamilyIndex
      present_queue_family_index,                         // uint32_t                               dstQueueFamilyIndex
      image_parameters.Handle,                            // VkImage                                image
      image_subresource_range                             // VkImageSubresourceRange                subresourceRange
    };
    vkCmdPipelineBarrier( command_buffer, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, 0, 0, nullptr, 0, nullptr, 1, &barrier_from_draw_to_present );

    if( vkEndCommandBuffer( command_buffer ) != VK_SUCCESS ) {
      std::cout << "Could not record command buffer!" << std::endl;
      return false;
    }
    return true;
  }

  bool Tutorial07::CreateFramebuffer( VkFramebuffer &framebuffer, VkImageView image_view ) {
    if( framebuffer != VK_NULL_HANDLE ) {
      vkDestroyFramebuffer( GetDevice(), framebuffer, nullptr );
      framebuffer = VK_NULL_HANDLE;
    }

    VkFramebufferCreateInfo framebuffer_create_info = {
      VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,      // VkStructureType                sType
      nullptr,                                        // const void                    *pNext
      0,                                              // VkFramebufferCreateFlags       flags
      Vulkan.RenderPass,                              // VkRenderPass                   renderPass
      1,                                              // uint32_t                       attachmentCount
      &image_view,                                    // const VkImageView             *pAttachments
      GetSwapChain().Extent.width,                    // uint32_t                       width
      GetSwapChain().Extent.height,                   // uint32_t                       height
      1                                               // uint32_t                       layers
    };

    if( vkCreateFramebuffer( GetDevice(), &framebuffer_create_info, nullptr, &framebuffer ) != VK_SUCCESS ) {
      std::cout << "Could not create a framebuffer!" << std::endl;
      return false;
    }

    return true;
  }

  bool Tutorial07::ChildOnWindowSizeChanged() {
    if( (GetDevice() != VK_NULL_HANDLE) &&
        (Vulkan.StagingBuffer.Handle != VK_NULL_HANDLE) ) {
      vkDeviceWaitIdle( GetDevice() );
      return CopyUniformBufferData();
    }
    return true;
  }

  bool Tutorial07::Draw() {
    static size_t           resource_index = 0;
    RenderingResourcesData &current_rendering_resource = Vulkan.RenderingResources[resource_index];
    VkSwapchainKHR          swap_chain = GetSwapChain().Handle;
    uint32_t                image_index;

    resource_index = (resource_index + 1) % VulkanTutorial07Parameters::ResourcesCount;

    if( vkWaitForFences( GetDevice(), 1, &current_rendering_resource.Fence, VK_FALSE, 1000000000 ) != VK_SUCCESS ) {
      std::cout << "Waiting for fence takes too long!" << std::endl;
      return false;
    }
    vkResetFences( GetDevice(), 1, &current_rendering_resource.Fence );

    VkResult result = vkAcquireNextImageKHR( GetDevice(), swap_chain, UINT64_MAX, current_rendering_resource.ImageAvailableSemaphore, VK_NULL_HANDLE, &image_index );
    switch( result ) {
      case VK_SUCCESS:
      case VK_SUBOPTIMAL_KHR:
        break;
      case VK_ERROR_OUT_OF_DATE_KHR:
        return OnWindowSizeChanged();
      default:
        std::cout << "Problem occurred during swap chain image acquisition!" << std::endl;
        return false;
    }

    if( !PrepareFrame( current_rendering_resource.CommandBuffer, GetSwapChain().Images[image_index], current_rendering_resource.Framebuffer ) ) {
      return false;
    }

    VkPipelineStageFlags wait_dst_stage_mask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    VkSubmitInfo submit_info = {
      VK_STRUCTURE_TYPE_SUBMIT_INFO,                          // VkStructureType              sType
      nullptr,                                                // const void                  *pNext
      1,                                                      // uint32_t                     waitSemaphoreCount
      &current_rendering_resource.ImageAvailableSemaphore,    // const VkSemaphore           *pWaitSemaphores
      &wait_dst_stage_mask,                                   // const VkPipelineStageFlags  *pWaitDstStageMask;
      1,                                                      // uint32_t                     commandBufferCount
      &current_rendering_resource.CommandBuffer,              // const VkCommandBuffer       *pCommandBuffers
      1,                                                      // uint32_t                     signalSemaphoreCount
      &current_rendering_resource.FinishedRenderingSemaphore  // const VkSemaphore           *pSignalSemaphores
    };

    if( vkQueueSubmit( GetGraphicsQueue().Handle, 1, &submit_info, current_rendering_resource.Fence ) != VK_SUCCESS ) {
      return false;
    }

    VkPresentInfoKHR present_info = {
      VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,                     // VkStructureType              sType
      nullptr,                                                // const void                  *pNext
      1,                                                      // uint32_t                     waitSemaphoreCount
      &current_rendering_resource.FinishedRenderingSemaphore, // const VkSemaphore           *pWaitSemaphores
      1,                                                      // uint32_t                     swapchainCount
      &swap_chain,                                            // const VkSwapchainKHR        *pSwapchains
      &image_index,                                           // const uint32_t              *pImageIndices
      nullptr                                                 // VkResult                    *pResults
    };
    result = vkQueuePresentKHR( GetPresentQueue().Handle, &present_info );

    switch( result ) {
      case VK_SUCCESS:
        break;
      case VK_ERROR_OUT_OF_DATE_KHR:
      case VK_SUBOPTIMAL_KHR:
        return OnWindowSizeChanged();
      default:
        std::cout << "Problem occurred during image presentation!" << std::endl;
        return false;
    }

    return true;
  }

  void Tutorial07::ChildClear() {
  }

  Tutorial07::~Tutorial07() {
    if( GetDevice() != VK_NULL_HANDLE ) {
      vkDeviceWaitIdle( GetDevice() );

      for( size_t i = 0; i < Vulkan.RenderingResources.size(); ++i ) {
        if( Vulkan.RenderingResources[i].Framebuffer != VK_NULL_HANDLE ) {
          vkDestroyFramebuffer( GetDevice(), Vulkan.RenderingResources[i].Framebuffer, nullptr );
        }
        if( Vulkan.RenderingResources[i].CommandBuffer != VK_NULL_HANDLE ) {
          vkFreeCommandBuffers( GetDevice(), Vulkan.CommandPool, 1, &Vulkan.RenderingResources[i].CommandBuffer );
        }
        if( Vulkan.RenderingResources[i].ImageAvailableSemaphore != VK_NULL_HANDLE ) {
          vkDestroySemaphore( GetDevice(), Vulkan.RenderingResources[i].ImageAvailableSemaphore, nullptr );
        }
        if( Vulkan.RenderingResources[i].FinishedRenderingSemaphore != VK_NULL_HANDLE ) {
          vkDestroySemaphore( GetDevice(), Vulkan.RenderingResources[i].FinishedRenderingSemaphore, nullptr );
        }
        if( Vulkan.RenderingResources[i].Fence != VK_NULL_HANDLE ) {
          vkDestroyFence( GetDevice(), Vulkan.RenderingResources[i].Fence, nullptr );
        }
      }

      if( Vulkan.CommandPool != VK_NULL_HANDLE ) {
        vkDestroyCommandPool( GetDevice(), Vulkan.CommandPool, nullptr );
        Vulkan.CommandPool = VK_NULL_HANDLE;
      }

      DestroyBuffer( Vulkan.VertexBuffer );

      DestroyBuffer( Vulkan.StagingBuffer );

      if( Vulkan.GraphicsPipeline != VK_NULL_HANDLE ) {
        vkDestroyPipeline( GetDevice(), Vulkan.GraphicsPipeline, nullptr );
        Vulkan.GraphicsPipeline = VK_NULL_HANDLE;
      }

      if( Vulkan.PipelineLayout != VK_NULL_HANDLE ) {
        vkDestroyPipelineLayout( GetDevice(), Vulkan.PipelineLayout, nullptr );
        Vulkan.PipelineLayout = VK_NULL_HANDLE;
      }

      if( Vulkan.RenderPass != VK_NULL_HANDLE ) {
        vkDestroyRenderPass( GetDevice(), Vulkan.RenderPass, nullptr );
        Vulkan.RenderPass = VK_NULL_HANDLE;
      }

      if( Vulkan.DescriptorSet.Pool != VK_NULL_HANDLE ) {
        vkDestroyDescriptorPool( GetDevice(), Vulkan.DescriptorSet.Pool, nullptr );
        Vulkan.DescriptorSet.Pool = VK_NULL_HANDLE;
      }

      if( Vulkan.DescriptorSet.Layout != VK_NULL_HANDLE ) {
        vkDestroyDescriptorSetLayout( GetDevice(), Vulkan.DescriptorSet.Layout, nullptr );
        Vulkan.DescriptorSet.Layout = VK_NULL_HANDLE;
      }

      DestroyBuffer( Vulkan.UniformBuffer );

      if( Vulkan.Image.Sampler != VK_NULL_HANDLE ) {
        vkDestroySampler( GetDevice(), Vulkan.Image.Sampler, nullptr );
        Vulkan.Image.Sampler = VK_NULL_HANDLE;
      }

      if( Vulkan.Image.View != VK_NULL_HANDLE ) {
        vkDestroyImageView( GetDevice(), Vulkan.Image.View, nullptr );
        Vulkan.Image.View = VK_NULL_HANDLE;
      }

      if( Vulkan.Image.Handle != VK_NULL_HANDLE ) {
        vkDestroyImage( GetDevice(), Vulkan.Image.Handle, nullptr );
        Vulkan.Image.Handle = VK_NULL_HANDLE;
      }

      if( Vulkan.Image.Memory != VK_NULL_HANDLE ) {
        vkFreeMemory( GetDevice(), Vulkan.Image.Memory, nullptr );
        Vulkan.Image.Memory = VK_NULL_HANDLE;
      }
    }
  }

  void Tutorial07::DestroyBuffer( BufferParameters& buffer ) {
    if( buffer.Handle != VK_NULL_HANDLE ) {
      vkDestroyBuffer( GetDevice(), buffer.Handle, nullptr );
      buffer.Handle = VK_NULL_HANDLE;
    }

    if( buffer.Memory != VK_NULL_HANDLE ) {
      vkFreeMemory( GetDevice(), buffer.Memory, nullptr );
      buffer.Memory = VK_NULL_HANDLE;
    }
  }

} // namespace ApiWithoutSecrets
