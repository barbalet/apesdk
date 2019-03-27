/****************************************************************
 
 NobleMTKView.m
 
 =============================================================
 
 Copyright 1996-2019 Tom Barbalet. All rights reserved.
 
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
 
 This software and Noble Ape are a continuing work of Tom Barbalet,
 begun on 13 June 1996. No apes or cats were harmed in the writing
 of this software.
 
 ****************************************************************/


#import "NobleMTKView.h"
#import "NobleMTKRenderer.h"

@implementation NobleMTKView
{
    NobleMTKRenderer *_renderer;
    NobleShared *_shared;
}

- (void)viewDidLoad
{


}

- (void) sharedReady
{
    [super sharedReady];
    // Set the view to use the default device
    self.device = MTLCreateSystemDefaultDevice();
    
    if(!self.device)
    {
        NSLog(@"Metal is not supported on this device");
        return;
    }
    
    _shared = self.shared;
    _renderer = [[NobleMTKRenderer alloc] initWithMetalKitView:self NobleShared:_shared];
    
    if(!_renderer)
    {
        NSLog(@"Renderer failed initialization");
        return;
    }
    
    // Initialize our renderer with the view size
    [_renderer mtkView:self drawableSizeWillChange:self.drawableSize];
    
    self.delegate = _renderer;
}

@end
