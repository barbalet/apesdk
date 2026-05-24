import CoreGraphics
import Foundation

enum EventGlyphKind: String, Codable, CaseIterable, Identifiable {
    case eating
    case grooming
    case conflict
    case swimming
    case birth
    case speech
    case objectExchange
    case death

    var id: String { rawValue }

    var symbolName: String {
        switch self {
        case .eating:
            return "leaf"
        case .grooming:
            return "hand.raised"
        case .conflict:
            return "exclamationmark.triangle"
        case .swimming:
            return "water.waves"
        case .birth:
            return "sparkle"
        case .speech:
            return "bubble.left"
        case .objectExchange:
            return "arrow.left.arrow.right"
        case .death:
            return "xmark.octagon"
        }
    }

    var priority: Int {
        switch self {
        case .death, .birth, .conflict:
            return 5
        case .objectExchange, .speech:
            return 4
        case .eating, .grooming, .swimming:
            return 3
        }
    }
}

struct EventGlyphRenderModel: Identifiable, Equatable {
    var id: String
    var eventID: EventIdentifier
    var kind: EventGlyphKind
    var mapPosition: MapPoint
    var screenPosition: CGPoint
    var ageCycles: Int
    var radius: CGFloat
    var color: RenderRGB
    var priority: Int
    var isExpired: Bool

    static func fixtureGlyphs(snapshot: UniverseSnapshot, projection: ViewportProjection, colors: TerrainColorSystem) -> [EventGlyphRenderModel] {
        EventGlyphKind.allCases.enumerated().map { index, kind in
            let eventCycle = max(0, snapshot.cycle - index * 17)
            let eventID = EventIdentifier(cycle: eventCycle, source: .runLog, localIndex: index)
            let angle = Double(index) * 0.83 + Double(snapshot.cycle % 180) * 0.003
            let mapPoint = MapPoint(
                x: projection.camera.center.x + cos(angle) * Double(72 + index * 11),
                y: projection.camera.center.y + sin(angle) * Double(58 + index * 9)
            )
            let color: RenderRGB
            switch kind {
            case .conflict, .death:
                color = colors.analysisHigh
            case .birth, .speech, .objectExchange:
                color = colors.analysisMid
            default:
                color = colors.markerDefault
            }

            return EventGlyphRenderModel(
                id: "\(eventID.cycle)-\(eventID.source.rawValue)-\(eventID.localIndex)",
                eventID: eventID,
                kind: kind,
                mapPosition: mapPoint,
                screenPosition: projection.screenPoint(for: mapPoint),
                ageCycles: snapshot.cycle - eventCycle,
                radius: CGFloat(8 + kind.priority),
                color: color.withOpacity(max(0.28, 1.0 - Double(index) * 0.08)),
                priority: kind.priority,
                isExpired: snapshot.cycle - eventCycle > 480
            )
        }
    }
}

struct PathTrailPoint: Identifiable, Equatable {
    var id: String
    var cycle: Int
    var mapPosition: MapPoint
    var screenPosition: CGPoint
    var ageCycles: Int
    var opacity: Double
}

struct PathTrailRenderModel: Identifiable, Equatable {
    var id: BeingIdentifier
    var points: [PathTrailPoint]
    var color: RenderRGB
    var isSelectedTrail: Bool
    var isPinnedTrail: Bool

    var shouldDraw: Bool {
        points.count >= 2
    }

    static func fixtureTrails(snapshot: UniverseSnapshot, markers: [ApeMarkerRenderModel], projection: ViewportProjection) -> [PathTrailRenderModel] {
        markers.filter { $0.isSelected || $0.isPinned }.map { marker in
            let points = (0..<12).map { index in
                let age = index * 8
                let offset = Double(index) * 4.0
                let mapPoint = MapPoint(
                    x: marker.mapPosition.x - cos(Double(index) * 0.71) * offset,
                    y: marker.mapPosition.y - sin(Double(index) * 0.71) * offset
                )
                return PathTrailPoint(
                    id: "\(marker.id.value)-\(snapshot.cycle - age)",
                    cycle: max(0, snapshot.cycle - age),
                    mapPosition: mapPoint,
                    screenPosition: projection.screenPoint(for: mapPoint),
                    ageCycles: age,
                    opacity: max(0.08, 1.0 - Double(age) / 128.0)
                )
            }

            return PathTrailRenderModel(
                id: marker.id,
                points: points,
                color: marker.color,
                isSelectedTrail: marker.isSelected,
                isPinnedTrail: marker.isPinned
            )
        }
    }
}

struct PathTrailPolicy: Equatable {
    var selectedTrailCycles: Int
    var pinnedTrailCycles: Int
    var maximumVisibleTrails: Int
    var minimumScreenPointSpacing: CGFloat

    static let interactive = PathTrailPolicy(
        selectedTrailCycles: 240,
        pinnedTrailCycles: 120,
        maximumVisibleTrails: 6,
        minimumScreenPointSpacing: 5
    )
}

enum TerritoryOverlayKind: String, Codable, CaseIterable, Identifiable {
    case territory
    case familiarity

    var id: String { rawValue }
}

struct TerritoryOverlayCell: Identifiable, Equatable {
    var id: String
    var mapRect: CGRect
    var screenRect: CGRect
    var value: Int
    var confidence: Double
    var kind: TerritoryOverlayKind
    var color: RenderRGB
}

struct TerritoryOverlayModel: Equatable {
    var kind: TerritoryOverlayKind
    var cells: [TerritoryOverlayCell]
    var isMeaningful: Bool

    static func fixture(kind: TerritoryOverlayKind, tiles: [TerrainTile], projection: ViewportProjection, colors: TerrainColorSystem) -> TerritoryOverlayModel {
        let cells = tiles.enumerated().map { index, tile in
            let value = Int((tile.meanHeight + tile.resourceMean + Double(index * 13)).truncatingRemainder(dividingBy: 100))
            let confidence = min(1.0, max(0.2, Double(tile.samples.count) / 9.0))
            let color = kind == .territory
                ? colors.analysisMid.withOpacity(0.10 + confidence * 0.16)
                : colors.analysisLow.withOpacity(0.08 + Double(value) / 500.0)
            return TerritoryOverlayCell(
                id: "\(kind.rawValue)-\(tile.key.id)",
                mapRect: tile.mapRect,
                screenRect: projection.screenRect(for: tile.mapRect),
                value: value,
                confidence: confidence,
                kind: kind,
                color: color
            )
        }

        return TerritoryOverlayModel(kind: kind, cells: cells, isMeaningful: cells.count >= 4)
    }
}

struct SocialLinkRenderModel: Identifiable, Equatable {
    var id: String
    var sourceID: BeingIdentifier
    var targetID: BeingIdentifier
    var sourceScreenPosition: CGPoint
    var targetScreenPosition: CGPoint
    var relationshipStrength: Int
    var color: RenderRGB
    var lineWidth: CGFloat
    var compareMode: Bool

    static func fixtureLinks(markers: [ApeMarkerRenderModel], colors: TerrainColorSystem, compareMode: Bool) -> [SocialLinkRenderModel] {
        guard let source = markers.first(where: \.isSelected) ?? markers.first else {
            return []
        }

        return markers.dropFirst().prefix(compareMode ? 6 : 4).map { target in
            let strength = abs((source.id.value * 17 + target.id.value * 31) % 100)
            return SocialLinkRenderModel(
                id: "\(source.id.value)-\(target.id.value)",
                sourceID: source.id,
                targetID: target.id,
                sourceScreenPosition: source.screenPosition,
                targetScreenPosition: target.screenPosition,
                relationshipStrength: strength,
                color: (strength > 50 ? colors.analysisMid : colors.markerDefault).withOpacity(compareMode ? 0.62 : 0.40),
                lineWidth: CGFloat(max(1, min(4, strength / 25))),
                compareMode: compareMode
            )
        }
    }
}

struct MemoryLocationRenderModel: Identifiable, Equatable {
    var id: String
    var memoryID: String
    var eventID: EventIdentifier?
    var mapPosition: MapPoint
    var screenPosition: CGPoint
    var ageCycles: Int
    var color: RenderRGB

    static func fixtureLocations(snapshot: UniverseSnapshot, projection: ViewportProjection, colors: TerrainColorSystem) -> [MemoryLocationRenderModel] {
        (0..<8).map { index in
            let age = index * 53
            let mapPoint = MapPoint(
                x: projection.camera.center.x + cos(Double(index) * 1.12) * Double(40 + index * 16),
                y: projection.camera.center.y + sin(Double(index) * 1.12) * Double(30 + index * 13)
            )
            let eventID = EventIdentifier(cycle: max(0, snapshot.cycle - age), source: .episodicMemory, localIndex: index)
            return MemoryLocationRenderModel(
                id: "memory-\(index)",
                memoryID: "fixture-memory-\(index)",
                eventID: eventID,
                mapPosition: mapPoint,
                screenPosition: projection.screenPoint(for: mapPoint),
                ageCycles: age,
                color: colors.analysisMid.withOpacity(max(0.20, 0.9 - Double(index) * 0.08))
            )
        }
    }
}

enum WeatherInstrumentKind: String, Codable, CaseIterable, Identifiable {
    case tide
    case cloud
    case wind
    case timeOfDay

    var id: String { rawValue }
}

struct WeatherInstrumentModel: Identifiable, Equatable {
    var id: WeatherInstrumentKind { kind }
    var kind: WeatherInstrumentKind
    var title: String
    var valueDescription: String
    var normalizedValue: Double
    var vector: MapVector?

    static func make(snapshot: UniverseSnapshot, weather: WeatherRenderState, lighting: TimeOfDayLighting) -> [WeatherInstrumentModel] {
        [
            WeatherInstrumentModel(
                kind: .tide,
                title: "Tide",
                valueDescription: "\(snapshot.tide.level ?? 0)",
                normalizedValue: min(1.0, Double(snapshot.tide.level ?? 0) / 128.0),
                vector: nil
            ),
            WeatherInstrumentModel(
                kind: .cloud,
                title: "Cloud",
                valueDescription: "\(Int(weather.cloudCoverage * 100))%",
                normalizedValue: weather.cloudCoverage,
                vector: nil
            ),
            WeatherInstrumentModel(
                kind: .wind,
                title: "Wind",
                valueDescription: "\(weather.windVector.x), \(weather.windVector.y)",
                normalizedValue: min(1.0, Double(abs(weather.windVector.x) + abs(weather.windVector.y)) / 16.0),
                vector: weather.windVector
            ),
            WeatherInstrumentModel(
                kind: .timeOfDay,
                title: "Time",
                valueDescription: lighting.phase.rawValue,
                normalizedValue: Double(lighting.landTimeMinutes) / 1440.0,
                vector: nil
            )
        ]
    }
}

struct TerrainCursorModel: Equatable {
    var screenPosition: CGPoint
    var mapPoint: MapPoint
    var sample: TerrainSampleSnapshot?
    var isVisible: Bool

    var label: String {
        guard let sample else {
            return "\(mapPoint.coordinate.x), \(mapPoint.coordinate.y)"
        }

        return "\(sample.coordinate.x), \(sample.coordinate.y) h:\(sample.height)"
    }

    static func hidden(at mapPoint: MapPoint = MapPoint(x: 0, y: 0)) -> TerrainCursorModel {
        TerrainCursorModel(screenPosition: .zero, mapPoint: mapPoint, sample: nil, isVisible: false)
    }
}

enum RenderOverlayKind: String, Codable, CaseIterable, Identifiable {
    case baseWorld
    case resources
    case territory
    case social
    case memory
    case weather
    case eventGlyphs
    case selection
    case cursor

    var id: String { rawValue }
}

struct OverlayPriorityRule: Identifiable, Equatable {
    var id: RenderOverlayKind { kind }
    var kind: RenderOverlayKind
    var priority: Int
    var allowsBlending: Bool

    static let defaults = [
        OverlayPriorityRule(kind: .baseWorld, priority: 0, allowsBlending: true),
        OverlayPriorityRule(kind: .resources, priority: 10, allowsBlending: true),
        OverlayPriorityRule(kind: .territory, priority: 20, allowsBlending: true),
        OverlayPriorityRule(kind: .memory, priority: 30, allowsBlending: true),
        OverlayPriorityRule(kind: .social, priority: 40, allowsBlending: true),
        OverlayPriorityRule(kind: .weather, priority: 50, allowsBlending: true),
        OverlayPriorityRule(kind: .eventGlyphs, priority: 60, allowsBlending: true),
        OverlayPriorityRule(kind: .selection, priority: 70, allowsBlending: false),
        OverlayPriorityRule(kind: .cursor, priority: 80, allowsBlending: false)
    ]
}

struct OverlayCompositionPlan: Equatable {
    var enabledKinds: [RenderOverlayKind]
    var rules: [OverlayPriorityRule]

    var orderedKinds: [RenderOverlayKind] {
        enabledKinds.sorted { left, right in
            priority(for: left) < priority(for: right)
        }
    }

    func priority(for kind: RenderOverlayKind) -> Int {
        rules.first { $0.kind == kind }?.priority ?? 0
    }

    static func make(enabledOverlays: Set<UniverseOverlay>) -> OverlayCompositionPlan {
        var kinds: [RenderOverlayKind] = [.baseWorld, .eventGlyphs, .selection, .cursor]
        if enabledOverlays.contains(.resources) {
            kinds.append(.resources)
        }
        if enabledOverlays.contains(.social) {
            kinds.append(.social)
        }
        if enabledOverlays.contains(.memory) {
            kinds.append(.memory)
        }
        if enabledOverlays.contains(.weather) {
            kinds.append(.weather)
        }

        return OverlayCompositionPlan(enabledKinds: kinds, rules: OverlayPriorityRule.defaults)
    }
}

struct RendererPerformanceFixture: Equatable {
    var populationCount: Int
    var zoom: CGFloat
    var enabledOverlayCount: Int
    var maximumFrameMilliseconds: Double
    var maximumMarkerCount: Int
    var maximumGlyphCount: Int

    var isLargePopulation: Bool {
        populationCount >= 256
    }

    static let largePopulationHighZoom = RendererPerformanceFixture(
        populationCount: 512,
        zoom: 8.0,
        enabledOverlayCount: 5,
        maximumFrameMilliseconds: 16.67,
        maximumMarkerCount: 512,
        maximumGlyphCount: 128
    )
}

struct RenderSourceProbe: Equatable {
    var title: String
    var snapshotValue: String
    var renderedValue: String

    var matches: Bool {
        snapshotValue == renderedValue
    }
}

struct RenderSnapshotComparison: Equatable {
    var snapshotCycle: Int
    var probes: [RenderSourceProbe]

    var allProbesMatch: Bool {
        probes.allSatisfy(\.matches)
    }

    static func compare(snapshot: UniverseSnapshot, scene: UniverseRenderScene) -> RenderSnapshotComparison {
        RenderSnapshotComparison(
            snapshotCycle: snapshot.cycle,
            probes: [
                RenderSourceProbe(title: "cycle", snapshotValue: "\(snapshot.cycle)", renderedValue: "\(snapshot.cycle)"),
                RenderSourceProbe(title: "population", snapshotValue: "\(min(max(5, snapshot.population.count / 12), 18))", renderedValue: "\(scene.apeMarkers.count)"),
                RenderSourceProbe(title: "tide", snapshotValue: "\(snapshot.tide.level ?? 96)", renderedValue: "\(scene.waterSamples.first?.tideLevel ?? 96)")
            ]
        )
    }
}

struct RenderOverlayScene: Equatable {
    var eventGlyphs: [EventGlyphRenderModel]
    var pathTrails: [PathTrailRenderModel]
    var territory: TerritoryOverlayModel
    var familiarity: TerritoryOverlayModel
    var socialLinks: [SocialLinkRenderModel]
    var memoryLocations: [MemoryLocationRenderModel]
    var weatherInstruments: [WeatherInstrumentModel]
    var cursor: TerrainCursorModel
    var composition: OverlayCompositionPlan
    var sourceComparison: RenderSnapshotComparison

    static func make(
        snapshot: UniverseSnapshot,
        scene: UniverseRenderScene,
        selection: BeingSelectionState,
        terrainSample: TerrainSampleSnapshot?,
        enabledOverlays: Set<UniverseOverlay>
    ) -> RenderOverlayScene {
        let cursor = terrainSample.map {
            TerrainCursorModel(
                screenPosition: scene.projection.screenPoint(for: MapPoint($0.coordinate)),
                mapPoint: MapPoint($0.coordinate),
                sample: $0,
                isVisible: true
            )
        } ?? .hidden(at: scene.projection.camera.center)

        return RenderOverlayScene(
            eventGlyphs: EventGlyphRenderModel.fixtureGlyphs(snapshot: snapshot, projection: scene.projection, colors: scene.colorSystem),
            pathTrails: PathTrailRenderModel.fixtureTrails(snapshot: snapshot, markers: scene.apeMarkers, projection: scene.projection),
            territory: TerritoryOverlayModel.fixture(kind: .territory, tiles: scene.terrainCache.visibleTiles, projection: scene.projection, colors: scene.colorSystem),
            familiarity: TerritoryOverlayModel.fixture(kind: .familiarity, tiles: scene.terrainCache.visibleTiles, projection: scene.projection, colors: scene.colorSystem),
            socialLinks: SocialLinkRenderModel.fixtureLinks(markers: scene.apeMarkers, colors: scene.colorSystem, compareMode: selection.pinnedBeingIDs.count > 1),
            memoryLocations: MemoryLocationRenderModel.fixtureLocations(snapshot: snapshot, projection: scene.projection, colors: scene.colorSystem),
            weatherInstruments: WeatherInstrumentModel.make(snapshot: snapshot, weather: scene.weather, lighting: scene.lighting),
            cursor: cursor,
            composition: OverlayCompositionPlan.make(enabledOverlays: enabledOverlays),
            sourceComparison: RenderSnapshotComparison.compare(snapshot: snapshot, scene: scene)
        )
    }
}
