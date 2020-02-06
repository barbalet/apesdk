/****************************************************************
 
 NobleMTKRenderer.m
 
 =============================================================
 
 Copyright 1996-2020 Tom Barbalet. All rights reserved.
 
 Permission is hereby granted, free of charge, to any person
 obtaining a copy of this software and associated documentation
 files (the "Software"), to deal in the Software without
 restriction, including without limitation the rights to use,
 copy, modify, merge, publish, distribute, sublicense, and/or
 sell copies of the Software, and to permit persons to whom the
 Software is furnished to do so, subject to the following
 conditions:
 
 The above copyright notice and this permission notice shall be
 included in all copies or substantial portions of the Software.
 
 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
 OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 OTHER DEALINGS IN THE SOFTWARE.
 
 This software is a continuing work of Tom Barbalet, begun on
 13 June 1996. No apes or cats were harmed in the writing of
 this software.
 
 ****************************************************************/


@import simd;
@import MetalKit;

#ifndef    _WIN32

#include "../toolkit/toolkit.h"
#include "../script/script.h"
#include "../sim/sim.h"

#else

#include "..\toolkit\toolkit.h"
#include "..\script\script.h"
#include "..\sim\sim.h"

#endif

#ifdef MUSHROOM
#import "NobleShared.h"
#else
#ifndef NOBLE_PLANET
#ifdef WARFARE
#import "Noble_Warfare-Swift.h"
#else
#import "Simulated_Ape-Swift.h"
#endif
#else
#import "NobleShared.h"
#endif
#endif

#import "NobleMTKRenderer.h"

// Header shared between C code here, which executes Metal API commands, and .metal files, which
//   uses these types as inputs to the shaders
#import "NobleMTKShaderTypes.h"

// Main class performing the rendering
@implementation NobleMTKRenderer
{
    // The device (aka GPU) we're using to render
    id<MTLDevice> _device;

    // Our render pipeline composed of our vertex and fragment shaders in the .metal shader file
    id<MTLRenderPipelineState> _pipelineState;

    // The command Queue from which we'll obtain command buffers
    id<MTLCommandQueue> _commandQueue;

    // The Metal texture object
    id<MTLTexture> _texture;

    // The Metal buffer in which we store our vertex data
    id<MTLBuffer> _vertices;

    // The number of vertices in our vertex buffer
    NSUInteger _numVertices;

    // The current size of our view so we can use this in our render pipeline
    vector_uint2 _viewportSize;
    
    NobleShared * _shared;
}


static unsigned char   outputBuffer[TERRAIN_WINDOW_AREA*4];


- (void) updateRenderedView:(nonnull MTKView *)view
{
    NSUInteger      dim_x = (NSUInteger)[view bounds].size.width;
    NSUInteger      dim_y = (NSUInteger)[view bounds].size.height;
    
    [_shared draw:outputBuffer width:(NSInteger)dim_x height:(NSInteger)dim_y];
    
    MTLTextureDescriptor *textureDescriptor = [[MTLTextureDescriptor alloc] init];
    
    // Indicate that each pixel has a blue, green, red, and alpha channel, where each channel is
    // an 8-bit unsigned normalized value (i.e. 0 maps to 0.0 and 255 maps to 1.0)
    textureDescriptor.pixelFormat = MTLPixelFormatBGRA8Unorm;
    // Set the pixel dimensions of the texture
    textureDescriptor.width = dim_x;
    textureDescriptor.height = dim_y;
    
    // Create the texture from the device by using the descriptor
    _texture = [_device newTextureWithDescriptor:textureDescriptor];
    
    // Calculate the number of bytes per row of our image.
    
    MTLRegion region = {
        { 0, 0, 0 },                   // MTLOrigin
        {dim_x, dim_y, 1} // MTLSize
    };
    
    // Copy the bytes from our data object into the texture
    [_texture replaceRegion:region
                mipmapLevel:0
                  withBytes:outputBuffer
                bytesPerRow:(4 * dim_x)];
    
    
    const float pos_x = dim_x;
    const float pos_y = dim_y;
    const float neg_x = 0 - pos_x;
    const float neg_y = 0 - pos_y;
    
    // Set up a simple MTLBuffer with our vertices which include texture coordinates
    const NobleMTKVertex quadVertices[] =
    {
        // Pixel positions, Texture coordinates
        { { pos_x,  neg_y },  { 1.f, 1.f } },
        { { neg_x,  neg_y },  { 0.f, 1.f } },
        { { neg_x,  pos_y },  { 0.f, 0.f } },
        
        { { pos_x,  neg_y },  { 1.f, 1.f } },
        { { neg_x,  pos_y },  { 0.f, 0.f } },
        { { pos_x,  pos_y },  { 1.f, 0.f } },
    };
    
    // Create our vertex buffer, and initialize it with our quadVertices array
    _vertices = [_device newBufferWithBytes:quadVertices
                                     length:sizeof(quadVertices)
                                    options:MTLResourceStorageModeShared];
    
    // Calculate the number of vertices by dividing the byte length by the size of each vertex
    _numVertices = sizeof(quadVertices) / sizeof(NobleMTKVertex);
    
    /// Create our render pipeline
    
    // Load all the shader files with a .metal file extension in the project
    id<MTLLibrary> defaultLibrary = [_device newDefaultLibrary];
    
    // Load the vertex function from the library
    id<MTLFunction> vertexFunction = [defaultLibrary newFunctionWithName:@"vertexShader"];
    
    // Load the fragment function from the library
    id<MTLFunction> fragmentFunction = [defaultLibrary newFunctionWithName:@"samplingShader"];
    
    // Set up a descriptor for creating a pipeline state object
    MTLRenderPipelineDescriptor *pipelineStateDescriptor = [[MTLRenderPipelineDescriptor alloc] init];
    pipelineStateDescriptor.label = @"Texturing Pipeline";
    pipelineStateDescriptor.vertexFunction = vertexFunction;
    pipelineStateDescriptor.fragmentFunction = fragmentFunction;
    pipelineStateDescriptor.colorAttachments[0].pixelFormat = view.colorPixelFormat;
    
    NSError *error = NULL;
    _pipelineState = [_device newRenderPipelineStateWithDescriptor:pipelineStateDescriptor
                                                             error:&error];
    if (!_pipelineState)
    {
        // Pipeline State creation could fail if we haven't properly set up our pipeline descriptor.
        //  If the Metal API validation is enabled, we can find out more information about what
        //  went wrong.  (Metal API validation is enabled by default when a debug build is run
        //  from Xcode)
        NSLog(@"Failed to created pipeline state, error %@", error);
    }
    
    // Create the command queue
    _commandQueue = [_device newCommandQueue];
}

/// Initialize with the MetalKit view from which we'll obtain our Metal device
- (nonnull instancetype)initWithMetalKitView:(nonnull MTKView *)mtkView
                                 NobleShared:(nonnull NobleShared *)shared
{
    self = [super init];
    if(self)
    {
        _device = mtkView.device;
        _shared = shared;
    }
    
    return self;
}

/// Called whenever view changes orientation or is resized
- (void)mtkView:(nonnull MTKView *)view drawableSizeWillChange:(CGSize)size
{
    // Save the size of the drawable as we'll pass these
    //   values to our vertex shader when we draw
    _viewportSize.x = (unsigned int)size.width;
    _viewportSize.y = (unsigned int)size.height;
}

/// Called whenever the view needs to render a frame
- (void)drawInMTKView:(nonnull MTKView *)view
{
    @autoreleasepool {
        [self updateRenderedView:view];

        // Create a new command buffer for each render pass to the current drawable
        id<MTLCommandBuffer> commandBuffer = [_commandQueue commandBuffer];
        commandBuffer.label = @"NobleApeCommand";
        
        // Obtain a renderPassDescriptor generated from the view's drawable textures
        MTLRenderPassDescriptor *renderPassDescriptor = view.currentRenderPassDescriptor;
        
        if(renderPassDescriptor != nil)
        {


            // Create a render command encoder so we can render into something
            id<MTLRenderCommandEncoder> renderEncoder =
            [commandBuffer renderCommandEncoderWithDescriptor:renderPassDescriptor];
            renderEncoder.label = @"NobleApeEncoder";
            
            // Set the region of the drawable to which we'll draw.
            [renderEncoder setViewport:(MTLViewport){0.0, 0.0, _viewportSize.x, _viewportSize.y, -1.0, 1.0 }];
            
            [renderEncoder setRenderPipelineState:_pipelineState];
            
            [renderEncoder setVertexBuffer:_vertices
                                    offset:0
                                   atIndex:NobleMTKVertexInputIndexVertices];
            
            [renderEncoder setVertexBytes:&_viewportSize
                                   length:sizeof(_viewportSize)
                                  atIndex:NobleMTKVertexInputIndexViewportSize];
            
            // Set the texture object.  The NobleMTKTextureIndexBaseColor enum value corresponds
            ///  to the 'colorMap' argument in our 'samplingShader' function because its
            //   texture attribute qualifier also uses AAPLTextureIndexBaseColor for its index
            [renderEncoder setFragmentTexture:_texture
                                      atIndex:NobleMTKTextureIndexBaseColor];
            
            // Draw the vertices of our triangles
            [renderEncoder drawPrimitives:MTLPrimitiveTypeTriangle
                              vertexStart:0
                              vertexCount:_numVertices];
            
            [renderEncoder endEncoding];
            
            // Schedule a present once the framebuffer is complete using the current drawable
            [commandBuffer presentDrawable:view.currentDrawable];

        }
        
        
        // Finalize rendering here & push the command buffer to the GPU
        [commandBuffer commit];
    }
}

@end
