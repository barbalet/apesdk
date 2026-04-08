/****************************************************************

 ASMacView.swift

 =============================================================

 Copyright 1996-2025 Tom Barbalet. All rights reserved.

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
 
 ****************************************************************/


import Cocoa
import simd

class ASMacView: SimulationMacView {
    override func makeShared(frame frameRect: NSRect, title: String) -> SimulationShared {
        ASShared(frame: frameRect, title: title)
    }
}

func immersiveApeClamp(_ value: Float, min minimum: Float, max maximum: Float) -> Float {
    Swift.min(Swift.max(value, minimum), maximum)
}

func immersiveApeSaturate(_ value: Float) -> Float {
    immersiveApeClamp(value, min: 0, max: 1)
}

func immersiveApeMix(_ lhs: SIMD3<Float>, _ rhs: SIMD3<Float>, t: Float) -> SIMD3<Float> {
    lhs + ((rhs - lhs) * immersiveApeSaturate(t))
}

func immersiveApeMix(_ lhs: SIMD4<Float>, _ rhs: SIMD4<Float>, t: Float) -> SIMD4<Float> {
    lhs + ((rhs - lhs) * immersiveApeSaturate(t))
}

func immersiveApeLerp(_ lhs: SIMD3<Float>, _ rhs: SIMD3<Float>, factor: Float) -> SIMD3<Float> {
    lhs + ((rhs - lhs) * immersiveApeSaturate(factor))
}

func immersiveApeHash(_ x: Int32, _ y: Int32, seed: UInt32) -> UInt32 {
    var value = UInt32(bitPattern: x) &* 73856093
    value ^= UInt32(bitPattern: y) &* 19349663
    value ^= seed &* 83492791
    value ^= value >> 13
    value &*= 1274126177
    value ^= value >> 16
    return value
}

func immersiveApeNoise(_ x: Int32, _ y: Int32, seed: UInt32) -> Float {
    Float(immersiveApeHash(x, y, seed: seed) & 0xffff) / Float(0xffff)
}

extension SIMD3 where Scalar == Float {
    var normalizedSafe: SIMD3<Float> {
        let lengthSquared = simd_length_squared(self)
        if lengthSquared < 0.00001 {
            return SIMD3<Float>(0, 1, 0)
        }
        return simd_normalize(self)
    }
}

extension simd_float4x4 {
    init(perspectiveFovY fovy: Float, aspect: Float, nearZ: Float, farZ: Float) {
        let yScale = 1 / tan(fovy * 0.5)
        let xScale = yScale / aspect
        let zRange = farZ - nearZ
        let zScale = farZ / zRange
        let wzScale = (-nearZ * farZ) / zRange

        self.init(
            SIMD4<Float>(xScale, 0, 0, 0),
            SIMD4<Float>(0, yScale, 0, 0),
            SIMD4<Float>(0, 0, zScale, 1),
            SIMD4<Float>(0, 0, wzScale, 0)
        )
    }

    init(lookAt eye: SIMD3<Float>, target: SIMD3<Float>, up: SIMD3<Float>) {
        let forward = (target - eye).normalizedSafe
        let right = simd_normalize(simd_cross(forward, up.normalizedSafe))
        let adjustedUp = simd_cross(right, forward)

        self.init(
            SIMD4<Float>(right.x, adjustedUp.x, -forward.x, 0),
            SIMD4<Float>(right.y, adjustedUp.y, -forward.y, 0),
            SIMD4<Float>(right.z, adjustedUp.z, -forward.z, 0),
            SIMD4<Float>(-simd_dot(right, eye), -simd_dot(adjustedUp, eye), simd_dot(forward, eye), 1)
        )
    }
}
