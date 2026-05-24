import CoreGraphics
import Foundation
import SwiftUI

enum UniverseCoordinateSystem: String, CaseIterable, Identifiable {
    case apeSDKMap
    case viewport
    case screen
    case panelGlyph

    var id: String { rawValue }

    var role: String {
        switch self {
        case .apeSDKMap:
            return "Integer ApeSDK map coordinates, with terrain and being state owned by the simulation."
        case .viewport:
            return "Camera-relative world coordinates used for pan, zoom, follow, and fit operations."
        case .screen:
            return "SwiftUI canvas coordinates measured in points inside the current viewport."
        case .panelGlyph:
            return "Small normalized coordinates used for inspector glyphs, legends, sparklines, and compact overlays."
        }
    }
}

struct CoordinateSystemDescriptor: Identifiable, Equatable {
    var id: UniverseCoordinateSystem { system }
    var system: UniverseCoordinateSystem
    var unitName: String
    var originDescription: String
    var handednessDescription: String

    static let defaults = [
        CoordinateSystemDescriptor(
            system: .apeSDKMap,
            unitName: "map cell",
            originDescription: "ApeSDK terrain origin",
            handednessDescription: "Integer x and y values following ApeSDK terrain conventions."
        ),
        CoordinateSystemDescriptor(
            system: .viewport,
            unitName: "map cell at zoom 1",
            originDescription: "Camera center",
            handednessDescription: "Positive x right, positive y down after projection."
        ),
        CoordinateSystemDescriptor(
            system: .screen,
            unitName: "point",
            originDescription: "Top-left canvas point",
            handednessDescription: "SwiftUI point coordinates."
        ),
        CoordinateSystemDescriptor(
            system: .panelGlyph,
            unitName: "normalized point",
            originDescription: "Top-left glyph bounds",
            handednessDescription: "0...1 values for compact panel rendering."
        )
    ]
}

struct MapPoint: Codable, Equatable {
    var x: Double
    var y: Double

    init(x: Double, y: Double) {
        self.x = x
        self.y = y
    }

    init(_ coordinate: MapCoordinate) {
        x = Double(coordinate.x)
        y = Double(coordinate.y)
    }

    var coordinate: MapCoordinate {
        MapCoordinate(x: Int(x.rounded()), y: Int(y.rounded()))
    }
}

struct RenderCameraState: Equatable {
    var center: MapPoint
    var zoom: CGFloat
    var followedBeingID: BeingIdentifier?
    var minimumZoom: CGFloat
    var maximumZoom: CGFloat

    func pannedBy(x deltaX: Double, y deltaY: Double) -> RenderCameraState {
        var copy = self
        copy.center = MapPoint(x: center.x + deltaX / Double(max(zoom, minimumZoom)), y: center.y + deltaY / Double(max(zoom, minimumZoom)))
        copy.followedBeingID = nil
        return copy
    }

    func zoomed(by multiplier: CGFloat, around anchor: MapPoint? = nil) -> RenderCameraState {
        var copy = self
        copy.zoom = min(maximumZoom, max(minimumZoom, zoom * multiplier))
        if let anchor {
            copy.center = anchor
        }
        return copy
    }

    func following(_ id: BeingIdentifier?, at point: MapPoint?) -> RenderCameraState {
        var copy = self
        copy.followedBeingID = id
        if let point {
            copy.center = point
        }
        return copy
    }

    func returningToSelection(_ point: MapPoint?) -> RenderCameraState {
        guard let point else {
            return self
        }

        var copy = self
        copy.center = point
        copy.zoom = max(zoom, 2.0)
        return copy
    }

    static func initial(map: MapMetadataSnapshot) -> RenderCameraState {
        RenderCameraState(
            center: MapPoint(x: Double(map.mapDimension) / 2.0, y: Double(map.mapDimension) / 2.0),
            zoom: 1.0,
            followedBeingID: nil,
            minimumZoom: 0.35,
            maximumZoom: 10.0
        )
    }

    static func fitPopulation(points: [MapPoint], map: MapMetadataSnapshot, viewport: CGSize) -> RenderCameraState {
        guard let first = points.first, viewport.width > 0, viewport.height > 0 else {
            return .initial(map: map)
        }

        var minX = first.x
        var maxX = first.x
        var minY = first.y
        var maxY = first.y

        for point in points.dropFirst() {
            minX = min(minX, point.x)
            maxX = max(maxX, point.x)
            minY = min(minY, point.y)
            maxY = max(maxY, point.y)
        }

        let width = max(1.0, maxX - minX)
        let height = max(1.0, maxY - minY)
        let zoom = min(viewport.width / CGFloat(width), viewport.height / CGFloat(height)) * 0.76

        return RenderCameraState(
            center: MapPoint(x: (minX + maxX) / 2.0, y: (minY + maxY) / 2.0),
            zoom: min(10.0, max(0.35, zoom)),
            followedBeingID: nil,
            minimumZoom: 0.35,
            maximumZoom: 10.0
        )
    }
}

struct ViewportProjection: Equatable {
    var map: MapMetadataSnapshot
    var camera: RenderCameraState
    var viewportSize: CGSize

    func screenPoint(for point: MapPoint) -> CGPoint {
        CGPoint(
            x: viewportSize.width / 2.0 + CGFloat(point.x - camera.center.x) * camera.zoom,
            y: viewportSize.height / 2.0 + CGFloat(point.y - camera.center.y) * camera.zoom
        )
    }

    func mapPoint(for screenPoint: CGPoint) -> MapPoint {
        MapPoint(
            x: camera.center.x + Double((screenPoint.x - viewportSize.width / 2.0) / max(camera.zoom, camera.minimumZoom)),
            y: camera.center.y + Double((screenPoint.y - viewportSize.height / 2.0) / max(camera.zoom, camera.minimumZoom))
        )
    }

    func screenRect(for mapRect: CGRect) -> CGRect {
        let topLeft = screenPoint(for: MapPoint(x: Double(mapRect.minX), y: Double(mapRect.minY)))
        let bottomRight = screenPoint(for: MapPoint(x: Double(mapRect.maxX), y: Double(mapRect.maxY)))
        return CGRect(
            x: min(topLeft.x, bottomRight.x),
            y: min(topLeft.y, bottomRight.y),
            width: abs(bottomRight.x - topLeft.x),
            height: abs(bottomRight.y - topLeft.y)
        )
    }
}

struct TerrainTileKey: Hashable, Identifiable {
    var x: Int
    var y: Int
    var levelOfDetail: Int

    var id: String {
        "\(levelOfDetail)-\(x)-\(y)"
    }
}

enum TerrainLayerKind: String, CaseIterable, Identifiable {
    case height
    case water
    case shoreline
    case ecology

    var id: String { rawValue }
}

struct TerrainTileSample: Equatable {
    var coordinate: MapCoordinate
    var height: Int
    var waterLevel: Int
    var resourceDensity: Int

    var isUnderWater: Bool {
        height < waterLevel
    }
}

struct TerrainTile: Identifiable, Equatable {
    var key: TerrainTileKey
    var mapRect: CGRect
    var samples: [TerrainTileSample]
    var layers: Set<TerrainLayerKind>

    var id: String { key.id }

    var meanHeight: Double {
        guard samples.isEmpty == false else {
            return 0.0
        }

        let total = samples.reduce(0) { $0 + $1.height }
        return Double(total) / Double(samples.count)
    }

    var waterCoverage: Double {
        guard samples.isEmpty == false else {
            return 0.0
        }

        let waterCount = samples.filter(\.isUnderWater).count
        return Double(waterCount) / Double(samples.count)
    }

    var resourceMean: Double {
        guard samples.isEmpty == false else {
            return 0.0
        }

        let total = samples.reduce(0) { $0 + $1.resourceDensity }
        return Double(total) / Double(samples.count)
    }
}

struct TerrainTileCache: Equatable {
    var tileSize: Int
    var maximumTileCount: Int
    private(set) var tiles: [TerrainTileKey: TerrainTile]

    init(tileSize: Int = 64, maximumTileCount: Int = 256, tiles: [TerrainTileKey: TerrainTile] = [:]) {
        self.tileSize = tileSize
        self.maximumTileCount = maximumTileCount
        self.tiles = tiles
    }

    var visibleTiles: [TerrainTile] {
        tiles.values.sorted { left, right in
            if left.key.y == right.key.y {
                return left.key.x < right.key.x
            }

            return left.key.y < right.key.y
        }
    }

    mutating func insert(_ tile: TerrainTile) {
        if tiles.count >= maximumTileCount, let firstKey = visibleTiles.first?.key {
            tiles.removeValue(forKey: firstKey)
        }

        tiles[tile.key] = tile
    }

    static func fixture(snapshot: UniverseSnapshot, viewport: CGSize) -> TerrainTileCache {
        var cache = TerrainTileCache(tileSize: 64, maximumTileCount: 64)
        let mapSize = max(1, snapshot.map.mapDimension)
        let tileSize = cache.tileSize
        let visibleColumns = max(4, min(8, Int(viewport.width / 140.0)))
        let visibleRows = max(3, min(6, Int(viewport.height / 140.0)))
        let startX = max(0, mapSize / 2 - (visibleColumns * tileSize) / 2)
        let startY = max(0, mapSize / 2 - (visibleRows * tileSize) / 2)

        for row in 0..<visibleRows {
            for column in 0..<visibleColumns {
                let originX = startX + column * tileSize
                let originY = startY + row * tileSize
                let key = TerrainTileKey(x: column, y: row, levelOfDetail: 0)
                let samples = makeFixtureSamples(originX: originX, originY: originY, tileSize: tileSize, snapshot: snapshot)
                let tile = TerrainTile(
                    key: key,
                    mapRect: CGRect(x: originX, y: originY, width: min(tileSize, mapSize - originX), height: min(tileSize, mapSize - originY)),
                    samples: samples,
                    layers: [.height, .water, .shoreline, .ecology]
                )
                cache.insert(tile)
            }
        }

        return cache
    }

    private static func makeFixtureSamples(originX: Int, originY: Int, tileSize: Int, snapshot: UniverseSnapshot) -> [TerrainTileSample] {
        let waterLevel = snapshot.tide.level ?? 96
        return [0, tileSize / 2, max(0, tileSize - 1)].flatMap { yOffset in
            [0, tileSize / 2, max(0, tileSize - 1)].map { xOffset in
                let x = originX + xOffset
                let y = originY + yOffset
                let height = abs((x * 13 + y * 7 + snapshot.cycle) % 180)
                let density = abs((x * 5 + y * 11 + snapshot.landTime) % 100)
                return TerrainTileSample(coordinate: MapCoordinate(x: x, y: y), height: height, waterLevel: waterLevel, resourceDensity: density)
            }
        }
    }
}

struct RenderRGB: Codable, Equatable {
    var red: Double
    var green: Double
    var blue: Double
    var opacity: Double

    var color: Color {
        Color(red: red, green: green, blue: blue, opacity: opacity)
    }

    func withOpacity(_ opacity: Double) -> RenderRGB {
        RenderRGB(red: red, green: green, blue: blue, opacity: opacity)
    }
}

enum TerrainColorBand: String, CaseIterable, Identifiable {
    case deepWater
    case shallowWater
    case wetShore
    case lowland
    case upland
    case ridge

    var id: String { rawValue }
}

struct TerrainColorSystem: Equatable {
    var deepWater: RenderRGB
    var shallowWater: RenderRGB
    var shore: RenderRGB
    var lowland: RenderRGB
    var upland: RenderRGB
    var ridge: RenderRGB
    var analysisLow: RenderRGB
    var analysisMid: RenderRGB
    var analysisHigh: RenderRGB
    var markerSelected: RenderRGB
    var markerDefault: RenderRGB

    func band(for sample: TerrainTileSample) -> TerrainColorBand {
        if sample.height < sample.waterLevel - 16 {
            return .deepWater
        }

        if sample.height < sample.waterLevel {
            return .shallowWater
        }

        if sample.height < sample.waterLevel + 10 {
            return .wetShore
        }

        if sample.height < 92 {
            return .lowland
        }

        if sample.height < 142 {
            return .upland
        }

        return .ridge
    }

    func color(for band: TerrainColorBand) -> RenderRGB {
        switch band {
        case .deepWater:
            return deepWater
        case .shallowWater:
            return shallowWater
        case .wetShore:
            return shore
        case .lowland:
            return lowland
        case .upland:
            return upland
        case .ridge:
            return ridge
        }
    }

    func color(for sample: TerrainTileSample) -> RenderRGB {
        color(for: band(for: sample))
    }

    func analysisColor(density: Int) -> RenderRGB {
        switch density {
        case ..<34:
            return analysisLow
        case 34..<67:
            return analysisMid
        default:
            return analysisHigh
        }
    }

    static let accessible = TerrainColorSystem(
        deepWater: RenderRGB(red: 0.06, green: 0.20, blue: 0.34, opacity: 1.0),
        shallowWater: RenderRGB(red: 0.12, green: 0.39, blue: 0.49, opacity: 1.0),
        shore: RenderRGB(red: 0.45, green: 0.44, blue: 0.30, opacity: 1.0),
        lowland: RenderRGB(red: 0.19, green: 0.43, blue: 0.28, opacity: 1.0),
        upland: RenderRGB(red: 0.38, green: 0.48, blue: 0.30, opacity: 1.0),
        ridge: RenderRGB(red: 0.54, green: 0.55, blue: 0.52, opacity: 1.0),
        analysisLow: RenderRGB(red: 0.20, green: 0.24, blue: 0.28, opacity: 1.0),
        analysisMid: RenderRGB(red: 0.80, green: 0.64, blue: 0.21, opacity: 1.0),
        analysisHigh: RenderRGB(red: 0.81, green: 0.22, blue: 0.26, opacity: 1.0),
        markerSelected: RenderRGB(red: 1.0, green: 0.86, blue: 0.20, opacity: 1.0),
        markerDefault: RenderRGB(red: 0.94, green: 0.96, blue: 0.92, opacity: 1.0)
    )
}

enum WaterRenderState: String, Codable, CaseIterable, Identifiable {
    case dry
    case shore
    case shallow
    case deep

    var id: String { rawValue }
}

struct WaterRenderSample: Codable, Equatable {
    var coordinate: MapCoordinate
    var tideLevel: Int
    var terrainHeight: Int
    var state: WaterRenderState
    var shoreBlend: Double
    var inspectableValues: [String: Int]

    static func evaluate(sample: TerrainTileSample, tideLevel: Int) -> WaterRenderSample {
        let delta = tideLevel - sample.height
        let state: WaterRenderState
        if delta > 18 {
            state = .deep
        } else if delta > 0 {
            state = .shallow
        } else if delta > -10 {
            state = .shore
        } else {
            state = .dry
        }

        return WaterRenderSample(
            coordinate: sample.coordinate,
            tideLevel: tideLevel,
            terrainHeight: sample.height,
            state: state,
            shoreBlend: min(1.0, max(0.0, Double(10 - abs(delta)) / 10.0)),
            inspectableValues: [
                "tide": tideLevel,
                "height": sample.height,
                "delta": delta
            ]
        )
    }
}

enum DayPhase: String, Codable, CaseIterable, Identifiable {
    case dawn
    case day
    case dusk
    case night

    var id: String { rawValue }
}

struct TimeOfDayLighting: Equatable {
    var landTimeMinutes: Int
    var phase: DayPhase
    var overlayOpacity: Double
    var tint: RenderRGB
    var shadowStrength: Double

    static func evaluate(landTimeMinutes: Int) -> TimeOfDayLighting {
        let minute = ((landTimeMinutes % 1440) + 1440) % 1440
        switch minute {
        case 300..<420:
            return TimeOfDayLighting(landTimeMinutes: minute, phase: .dawn, overlayOpacity: 0.16, tint: RenderRGB(red: 0.95, green: 0.61, blue: 0.34, opacity: 0.18), shadowStrength: 0.30)
        case 420..<1020:
            return TimeOfDayLighting(landTimeMinutes: minute, phase: .day, overlayOpacity: 0.0, tint: RenderRGB(red: 1.0, green: 1.0, blue: 1.0, opacity: 0.0), shadowStrength: 0.12)
        case 1020..<1140:
            return TimeOfDayLighting(landTimeMinutes: minute, phase: .dusk, overlayOpacity: 0.18, tint: RenderRGB(red: 0.88, green: 0.43, blue: 0.29, opacity: 0.20), shadowStrength: 0.36)
        default:
            return TimeOfDayLighting(landTimeMinutes: minute, phase: .night, overlayOpacity: 0.34, tint: RenderRGB(red: 0.04, green: 0.08, blue: 0.18, opacity: 0.36), shadowStrength: 0.56)
        }
    }
}

struct WeatherRenderState: Equatable {
    var cloudCoverage: Double
    var rainIntensity: Double
    var windVector: MapVector
    var lightningVisible: Bool
    var transitionKey: String

    static func evaluate(snapshot: WeatherSnapshot, cycle: Int) -> WeatherRenderState {
        let condition = snapshot.condition ?? 0
        let cloudCoverage = min(1.0, Double((condition * 13 + cycle) % 100) / 100.0)
        let rainIntensity = condition > 7 ? min(1.0, Double(condition - 7) / 4.0) : 0.0
        let wind = snapshot.wind ?? MapVector(x: 0, y: 0)

        return WeatherRenderState(
            cloudCoverage: cloudCoverage,
            rainIntensity: rainIntensity,
            windVector: wind,
            lightningVisible: snapshot.lightningActive,
            transitionKey: "\(condition)-\(cycle / 60)"
        )
    }
}

enum ResourceRenderMode: String, Codable, CaseIterable, Identifiable {
    case normal
    case analysis

    var id: String { rawValue }
}

struct ResourceVisualizationHint: Identifiable, Equatable {
    var id: String
    var coordinate: MapCoordinate
    var density: Int
    var mode: ResourceRenderMode
    var radius: CGFloat
    var color: RenderRGB
    var label: String?
}

struct ResourceLegendEntry: Identifiable, Equatable {
    var id: String
    var title: String
    var rangeDescription: String
    var color: RenderRGB
}

struct ResourceVisualizationPlan: Equatable {
    var mode: ResourceRenderMode
    var hints: [ResourceVisualizationHint]
    var legend: [ResourceLegendEntry]

    static func make(mode: ResourceRenderMode, tiles: [TerrainTile], colors: TerrainColorSystem) -> ResourceVisualizationPlan {
        let hints = tiles.flatMap { tile in
            tile.samples.filter { $0.resourceDensity > (mode == .normal ? 70 : 20) }.prefix(mode == .normal ? 2 : 5).map { sample in
                let color = mode == .normal
                    ? colors.lowland.withOpacity(0.42)
                    : colors.analysisColor(density: sample.resourceDensity).withOpacity(0.72)
                return ResourceVisualizationHint(
                    id: "\(mode.rawValue)-\(sample.coordinate.x)-\(sample.coordinate.y)",
                    coordinate: sample.coordinate,
                    density: sample.resourceDensity,
                    mode: mode,
                    radius: mode == .normal ? 3.0 : 7.0,
                    color: color,
                    label: mode == .analysis ? "\(sample.resourceDensity)" : nil
                )
            }
        }

        let legend = [
            ResourceLegendEntry(id: "low", title: "Low", rangeDescription: "0-33", color: colors.analysisLow),
            ResourceLegendEntry(id: "medium", title: "Medium", rangeDescription: "34-66", color: colors.analysisMid),
            ResourceLegendEntry(id: "high", title: "High", rangeDescription: "67-100", color: colors.analysisHigh)
        ]

        return ResourceVisualizationPlan(mode: mode, hints: hints, legend: mode == .analysis ? legend : [])
    }
}

enum ApeMovementState: String, Codable, CaseIterable, Identifiable {
    case still
    case walking
    case turning
    case unknown

    var id: String { rawValue }
}

struct ApeMarkerRenderModel: Identifiable, Equatable {
    var id: BeingIdentifier
    var mapPosition: MapPoint
    var screenPosition: CGPoint
    var facingDegrees: Double
    var movementState: ApeMovementState
    var isSelected: Bool
    var isPinned: Bool
    var radius: CGFloat
    var label: String
    var labelVisible: Bool
    var color: RenderRGB

    static func fixtureMarkers(snapshot: UniverseSnapshot, selection: BeingSelectionState, projection: ViewportProjection, colors: TerrainColorSystem) -> [ApeMarkerRenderModel] {
        let count = min(max(5, snapshot.population.count / 12), 18)
        return (0..<count).map { index in
            let stableKey = BeingStableKey(genderName: UInt16(index % 2), familyName: UInt16(100 + index), dateOfBirth: index * 1440)
            let id = BeingIdentifier(value: index, stableKey: stableKey)
            let angle = Double(index) * 2.399963 + Double(snapshot.cycle % 360) * 0.002
            let radius = 46.0 + Double((index * 17) % 150)
            let mapPosition = MapPoint(
                x: projection.camera.center.x + cos(angle) * radius,
                y: projection.camera.center.y + sin(angle) * radius
            )
            let selected = selection.selectedBeingID == id || (selection.selectedBeingID == nil && index == 0)
            let pinned = selection.pinnedBeingIDs.contains(id)
            let markerRadius = max(4.0, min(11.0, 5.0 + projection.camera.zoom * 0.9))

            return ApeMarkerRenderModel(
                id: id,
                mapPosition: mapPosition,
                screenPosition: projection.screenPoint(for: mapPosition),
                facingDegrees: Double((index * 29 + snapshot.cycle) % 360),
                movementState: index.isMultiple(of: 5) ? .turning : (index.isMultiple(of: 2) ? .walking : .still),
                isSelected: selected,
                isPinned: pinned,
                radius: markerRadius,
                label: "Ape \(index + 1)",
                labelVisible: selected || pinned || projection.camera.zoom >= 2.4,
                color: selected ? colors.markerSelected : colors.markerDefault.withOpacity(0.88)
            )
        }
    }
}

struct UniverseRenderScene: Equatable {
    var coordinateSystems: [CoordinateSystemDescriptor]
    var projection: ViewportProjection
    var terrainCache: TerrainTileCache
    var colorSystem: TerrainColorSystem
    var waterSamples: [WaterRenderSample]
    var lighting: TimeOfDayLighting
    var weather: WeatherRenderState
    var resources: ResourceVisualizationPlan
    var apeMarkers: [ApeMarkerRenderModel]

    static func make(
        snapshot: UniverseSnapshot,
        selection: BeingSelectionState,
        viewportSize: CGSize,
        resourceMode: ResourceRenderMode
    ) -> UniverseRenderScene {
        let terrainCache = TerrainTileCache.fixture(snapshot: snapshot, viewport: viewportSize)
        let camera = RenderCameraState.initial(map: snapshot.map)
        let projection = ViewportProjection(map: snapshot.map, camera: camera, viewportSize: viewportSize)
        let colors = TerrainColorSystem.accessible
        let tideLevel = snapshot.tide.level ?? 96
        let waterSamples = terrainCache.visibleTiles.flatMap { tile in
            tile.samples.map { WaterRenderSample.evaluate(sample: $0, tideLevel: tideLevel) }
        }
        let resources = ResourceVisualizationPlan.make(mode: resourceMode, tiles: terrainCache.visibleTiles, colors: colors)
        let markers = ApeMarkerRenderModel.fixtureMarkers(snapshot: snapshot, selection: selection, projection: projection, colors: colors)

        return UniverseRenderScene(
            coordinateSystems: CoordinateSystemDescriptor.defaults,
            projection: projection,
            terrainCache: terrainCache,
            colorSystem: colors,
            waterSamples: waterSamples,
            lighting: TimeOfDayLighting.evaluate(landTimeMinutes: snapshot.landTime),
            weather: WeatherRenderState.evaluate(snapshot: snapshot.weather, cycle: snapshot.cycle),
            resources: resources,
            apeMarkers: markers
        )
    }
}
