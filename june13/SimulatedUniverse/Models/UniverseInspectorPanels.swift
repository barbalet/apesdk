import Foundation

struct CompactApeProfilePanel: Identifiable, Equatable {
    var id: BeingIdentifier
    var title: String
    var subtitle: String
    var metrics: [InspectorMetric]
    var location: MapCoordinate
    var sourceCycle: Int

    init(snapshot: BeingSnapshot, sourceCycle: Int) {
        id = snapshot.id
        title = snapshot.displayName
        subtitle = "\(snapshot.sex.rawValue) - \(snapshot.stateDescription)"
        metrics = [
            InspectorMetric(id: "age", label: "Age", value: "\(snapshot.ageInMinutes / 1440)d"),
            InspectorMetric(id: "energy", label: "Energy", value: "\(snapshot.energy)"),
            InspectorMetric(id: "hunger", label: "Hunger", value: "\(snapshot.drives.hunger)"),
            InspectorMetric(id: "social", label: "Social", value: "\(snapshot.drives.social)"),
            InspectorMetric(id: "inventory", label: "Inventory", value: "\(snapshot.inventory.leftHand)/\(snapshot.inventory.rightHand)")
        ]
        location = snapshot.location
        self.sourceCycle = sourceCycle
    }
}

struct InspectorMetric: Identifiable, Equatable {
    var id: String
    var label: String
    var value: String
}

enum ProfileSectionKind: String, Codable, CaseIterable, Identifiable {
    case bodyState
    case inventory
    case genetics
    case family
    case currentLocation

    var id: String { rawValue }

    var title: String {
        switch self {
        case .bodyState:
            return "Body State"
        case .inventory:
            return "Inventory"
        case .genetics:
            return "Genetics"
        case .family:
            return "Family"
        case .currentLocation:
            return "Current Location"
        }
    }
}

struct ExpandableProfileSection: Identifiable, Equatable {
    var id: ProfileSectionKind { kind }
    var kind: ProfileSectionKind
    var title: String
    var rows: [InspectorMetric]
    var isExpandedByDefault: Bool

    static func sections(for snapshot: BeingSnapshot) -> [ExpandableProfileSection] {
        [
            ExpandableProfileSection(
                kind: .bodyState,
                title: ProfileSectionKind.bodyState.title,
                rows: [
                    InspectorMetric(id: "state-flags", label: "State Flags", value: "\(snapshot.stateFlags)"),
                    InspectorMetric(id: "facing", label: "Facing", value: "\(snapshot.facing)"),
                    InspectorMetric(id: "energy", label: "Energy", value: "\(snapshot.energy)")
                ],
                isExpandedByDefault: true
            ),
            ExpandableProfileSection(
                kind: .inventory,
                title: ProfileSectionKind.inventory.title,
                rows: [
                    InspectorMetric(id: "left-hand", label: "Left Hand", value: "\(snapshot.inventory.leftHand)"),
                    InspectorMetric(id: "right-hand", label: "Right Hand", value: "\(snapshot.inventory.rightHand)"),
                    InspectorMetric(id: "slots", label: "Raw Slots", value: snapshot.inventory.rawSlots.map(String.init).joined(separator: ", "))
                ],
                isExpandedByDefault: false
            ),
            ExpandableProfileSection(
                kind: .genetics,
                title: ProfileSectionKind.genetics.title,
                rows: [
                    InspectorMetric(id: "gender-name", label: "Gender Name", value: snapshot.id.stableKey.map { "\($0.genderName)" } ?? "--"),
                    InspectorMetric(id: "family-name", label: "Family Name", value: snapshot.id.stableKey.map { "\($0.familyName)" } ?? "--")
                ],
                isExpandedByDefault: false
            ),
            ExpandableProfileSection(
                kind: .family,
                title: ProfileSectionKind.family.title,
                rows: [
                    InspectorMetric(id: "family-count", label: "Family Count", value: "\(snapshot.social.familyCount)"),
                    InspectorMetric(id: "known-count", label: "Known Count", value: "\(snapshot.social.knownCount)")
                ],
                isExpandedByDefault: false
            ),
            ExpandableProfileSection(
                kind: .currentLocation,
                title: ProfileSectionKind.currentLocation.title,
                rows: [
                    InspectorMetric(id: "location", label: "Location", value: "\(snapshot.location.x), \(snapshot.location.y)"),
                    InspectorMetric(id: "high-res", label: "High Resolution", value: snapshot.highResolutionLocation.map { "\($0.x), \($0.y)" } ?? "--")
                ],
                isExpandedByDefault: true
            )
        ]
    }
}

enum DriveKind: String, Codable, CaseIterable, Identifiable {
    case hunger
    case social
    case fatigue
    case sex

    var id: String { rawValue }
}

struct DriveTimelineSample: Identifiable, Equatable {
    var id: String
    var cycle: Int
    var values: [DriveKind: Int]

    func normalizedValue(for kind: DriveKind, scale: DriveTimelineScale) -> Double {
        let raw = values[kind] ?? 0
        return min(1.0, max(0.0, Double(raw - scale.minimum) / Double(max(1, scale.maximum - scale.minimum))))
    }
}

struct DriveTimelineScale: Equatable {
    var minimum: Int
    var maximum: Int

    static let apeSDKDefault = DriveTimelineScale(minimum: 0, maximum: 255)
}

struct DriveTimelineRetentionPolicy: Equatable {
    var sampleCadenceCycles: Int
    var maximumSamples: Int
    var maximumMemoryBytes: Int

    func shouldSample(lastSampleCycle: Int?, currentCycle: Int) -> Bool {
        guard let lastSampleCycle else {
            return true
        }

        return currentCycle - lastSampleCycle >= sampleCadenceCycles
    }

    static let interactive = DriveTimelineRetentionPolicy(sampleCadenceCycles: 8, maximumSamples: 1_800, maximumMemoryBytes: 256_000)
}

struct DriveTimelinePanelModel: Equatable {
    var beingID: BeingIdentifier
    var samples: [DriveTimelineSample]
    var scale: DriveTimelineScale
    var retentionPolicy: DriveTimelineRetentionPolicy

    mutating func append(snapshot: BeingSnapshot, cycle: Int) {
        guard retentionPolicy.shouldSample(lastSampleCycle: samples.last?.cycle, currentCycle: cycle) else {
            return
        }

        let sample = DriveTimelineSample(
            id: "\(snapshot.id.value)-\(cycle)",
            cycle: cycle,
            values: [
                .hunger: snapshot.drives.hunger,
                .social: snapshot.drives.social,
                .fatigue: snapshot.drives.fatigue,
                .sex: snapshot.drives.sex
            ]
        )
        samples.append(sample)
        if samples.count > retentionPolicy.maximumSamples {
            samples.removeFirst(samples.count - retentionPolicy.maximumSamples)
        }
    }
}

enum MemoryFilterKind: String, Codable, CaseIterable, Identifiable {
    case selfEvent
    case social
    case food
    case conflict
    case mating
    case object
    case water
    case movement

    var id: String { rawValue }
}

struct MemoryPanelRow: Identifiable, Equatable {
    var id: String
    var memoryID: String
    var eventType: String
    var timeDescription: String
    var affect: Int
    var location: MapCoordinate?
    var argument: Int
    var involvedBeingID: BeingIdentifier?
    var filterKinds: Set<MemoryFilterKind>

    init(memory: MemorySnapshot, cycle: Int) {
        id = memory.id
        memoryID = memory.id
        eventType = memory.eventDescription
        timeDescription = "Cycle \(cycle)"
        affect = memory.affectRaw
        location = memory.coordinate
        argument = memory.argumentRaw
        involvedBeingID = memory.involvedBeingID
        filterKinds = MemoryPanelRow.classify(memory)
    }

    static func classify(_ memory: MemorySnapshot) -> Set<MemoryFilterKind> {
        let lower = memory.eventDescription.lowercased()
        var filters: Set<MemoryFilterKind> = [.selfEvent]
        if memory.involvedBeingID != nil {
            filters.insert(.social)
        }
        if lower.contains("food") || lower.contains("eat") {
            filters.insert(.food)
        }
        if lower.contains("conflict") || lower.contains("fight") {
            filters.insert(.conflict)
        }
        if lower.contains("mate") || lower.contains("sex") {
            filters.insert(.mating)
        }
        if lower.contains("object") || lower.contains("carry") {
            filters.insert(.object)
        }
        if lower.contains("water") || lower.contains("swim") {
            filters.insert(.water)
        }
        if lower.contains("move") || lower.contains("walk") {
            filters.insert(.movement)
        }
        return filters
    }
}

struct MemoryFilterState: Equatable {
    var enabledFilters: Set<MemoryFilterKind>

    func includes(_ row: MemoryPanelRow) -> Bool {
        enabledFilters.isEmpty || row.filterKinds.isDisjoint(with: enabledFilters) == false
    }

    static let all = MemoryFilterState(enabledFilters: Set(MemoryFilterKind.allCases))
}

struct MemoryPanelModel: Equatable {
    var sourceBeingID: BeingIdentifier
    var rows: [MemoryPanelRow]
    var filters: MemoryFilterState

    var visibleRows: [MemoryPanelRow] {
        rows.filter(filters.includes)
    }
}

struct MemoryMapLink: Identifiable, Equatable {
    var id: String
    var memoryID: String
    var coordinate: MapCoordinate
    var overlayID: String
    var revealZoom: Double

    init?(row: MemoryPanelRow) {
        guard let location = row.location else {
            return nil
        }

        id = "memory-map-\(row.memoryID)"
        memoryID = row.memoryID
        coordinate = location
        overlayID = "memory-overlay-\(row.memoryID)"
        revealZoom = 3.0
    }
}

struct MemorySocialLink: Identifiable, Equatable {
    var id: String
    var memoryID: String
    var involvedBeingID: BeingIdentifier
    var relationshipEntryID: String

    init?(row: MemoryPanelRow) {
        guard let involvedBeingID = row.involvedBeingID else {
            return nil
        }

        id = "memory-social-\(row.memoryID)-\(involvedBeingID.value)"
        memoryID = row.memoryID
        self.involvedBeingID = involvedBeingID
        relationshipEntryID = "relationship-\(involvedBeingID.value)"
    }
}

struct SocialGraphRow: Identifiable, Equatable {
    var id: String
    var targetBeingID: BeingIdentifier?
    var targetName: String
    var relationshipType: String
    var attraction: Int
    var familiarity: Int
    var friendFoe: Int
    var evidenceEventIDs: [EventIdentifier]

    init(snapshot: SocialSnapshot) {
        id = snapshot.id
        targetBeingID = snapshot.targetBeingID
        targetName = snapshot.targetName
        relationshipType = snapshot.relationshipDescription
        attraction = snapshot.attraction
        familiarity = snapshot.familiarity
        friendFoe = snapshot.friendFoe
        evidenceEventIDs = snapshot.evidenceEventIDs
    }
}

struct SocialGraphPanelModel: Equatable {
    var sourceBeingID: BeingIdentifier
    var rows: [SocialGraphRow]

    var knownBeingCount: Int {
        rows.count
    }

    var strongestRelationship: SocialGraphRow? {
        rows.max { left, right in
            abs(left.attraction) + abs(left.familiarity) < abs(right.attraction) + abs(right.familiarity)
        }
    }
}

struct SocialEvidenceDrillDown: Identifiable, Equatable {
    var id: String
    var relationshipRowID: String
    var targetName: String
    var relevantMemories: [MemoryPanelRow]
    var evidenceEventIDs: [EventIdentifier]

    static func make(row: SocialGraphRow, memories: [MemoryPanelRow]) -> SocialEvidenceDrillDown {
        let relevant = memories.filter { memory in
            guard let targetID = row.targetBeingID else {
                return false
            }

            return memory.involvedBeingID == targetID || row.evidenceEventIDs.contains { $0 == EventIdentifier(cycle: Int(memory.timeDescription.filter(\.isNumber)) ?? -1, source: .episodicMemory, localIndex: 0) }
        }

        return SocialEvidenceDrillDown(
            id: "drilldown-\(row.id)",
            relationshipRowID: row.id,
            targetName: row.targetName,
            relevantMemories: relevant,
            evidenceEventIDs: row.evidenceEventIDs
        )
    }
}

struct InspectorPanelFixtureFactory {
    static func beingSnapshot(id: BeingIdentifier = BeingIdentifier(value: 0), cycle: Int = 0) -> BeingSnapshot {
        BeingSnapshot(
            id: id,
            transientIndex: id.value,
            displayName: "Ape \(id.value + 1)",
            location: MapCoordinate(x: 256, y: 256),
            highResolutionLocation: MapCoordinate(x: 2048, y: 2048),
            facing: cycle % 360,
            ageInMinutes: 52_000,
            sex: .female,
            energy: 612,
            stateFlags: 0,
            stateDescription: "Foraging",
            drives: DriveSnapshot(hunger: 41, social: 24, fatigue: 17, sex: 8, rawValues: [:]),
            inventory: InventorySnapshot(leftHand: 0, rightHand: 1, rawSlots: [0, 1]),
            social: SocialSummarySnapshot(knownCount: 7, familyCount: 3, strongestAttraction: 68, respectMean: 12)
        )
    }

    static func memories(source: BeingIdentifier) -> [MemorySnapshot] {
        (0..<6).map { index in
            MemorySnapshot(
                id: "memory-\(index)",
                sourceBeingID: source,
                sourceMemoryIndex: index,
                eventRaw: index,
                eventDescription: index.isMultiple(of: 2) ? "food encounter" : "social movement",
                affectRaw: 10 + index * 8,
                coordinate: MapCoordinate(x: 240 + index * 6, y: 260 - index * 4),
                argumentRaw: index * 3,
                involvedBeingID: index.isMultiple(of: 2) ? nil : BeingIdentifier(value: index + 1),
                evidenceEventID: EventIdentifier(cycle: 100 + index, source: .episodicMemory, localIndex: index)
            )
        }
    }

    static func social(source: BeingIdentifier) -> [SocialSnapshot] {
        (0..<5).map { index in
            SocialSnapshot(
                id: "relationship-\(index + 1)",
                sourceBeingID: source,
                targetBeingID: BeingIdentifier(value: index + 1),
                targetName: "Ape \(index + 2)",
                relationshipRaw: index,
                relationshipDescription: index.isMultiple(of: 2) ? "familiar" : "uncertain",
                friendFoe: 50 - index * 12,
                attraction: 20 + index * 9,
                familiarity: 30 + index * 7,
                lastKnownLocation: MapCoordinate(x: 200 + index * 14, y: 220 + index * 11),
                evidenceEventIDs: [EventIdentifier(cycle: 100 + index, source: .episodicMemory, localIndex: index)]
            )
        }
    }
}
