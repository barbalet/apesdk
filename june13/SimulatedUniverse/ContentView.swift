import SwiftUI

struct ContentView: View {
    @StateObject private var runtime = UniverseRuntimeController()

    var body: some View {
        NavigationSplitView {
            List {
                Section("Modes") {
                    ForEach(UniverseMode.allCases) { mode in
                        Label(mode.title, systemImage: modeIcon(for: mode))
                    }
                }

                Section("Overlays") {
                    ForEach(UniverseOverlay.allCases) { overlay in
                        Label(overlay.title, systemImage: overlayIcon(for: overlay))
                    }
                }
            }
            .navigationTitle("Simulated Universe")
        } detail: {
            HStack(spacing: 0) {
                WorldCanvas(snapshot: runtime.snapshot, phase: runtime.phase, selection: runtime.selection, terrainSample: runtime.terrainSample)

                Divider()

                InspectorPanel(snapshot: runtime.snapshot, metadata: runtime.metadata, health: runtime.health)
                    .frame(width: 340)
            }
            .toolbar {
                ToolbarItemGroup(placement: .primaryAction) {
                    Button {
                        runtime.pause()
                    } label: {
                        Label("Pause", systemImage: "pause.fill")
                    }

                    Button {
                        runtime.step()
                    } label: {
                        Label("Step", systemImage: "forward.frame.fill")
                    }

                    Button {
                        runtime.resume()
                    } label: {
                        Label("Run", systemImage: "play.fill")
                    }
                }
            }
            .onAppear {
                runtime.initialize()
            }
        }
    }

    private func modeIcon(for mode: UniverseMode) -> String {
        switch mode {
        case .explore:
            return "map"
        case .inspect:
            return "magnifyingglass"
        case .experiment:
            return "slider.horizontal.3"
        case .replay:
            return "clock.arrow.circlepath"
        }
    }

    private func overlayIcon(for overlay: UniverseOverlay) -> String {
        switch overlay {
        case .social:
            return "point.3.connected.trianglepath.dotted"
        case .memory:
            return "brain.head.profile"
        case .weather:
            return "cloud.sun.rain"
        case .resources:
            return "leaf"
        }
    }
}

private struct WorldCanvas: View {
    let snapshot: UniverseSnapshot
    let phase: RuntimePhase
    let selection: BeingSelectionState
    let terrainSample: TerrainSampleSnapshot?

    var body: some View {
        Canvas { context, size in
            let rect = CGRect(origin: .zero, size: size)
            let scene = UniverseRenderScene.make(
                snapshot: snapshot,
                selection: selection,
                viewportSize: size,
                resourceMode: .normal
            )
            let overlays = RenderOverlayScene.make(
                snapshot: snapshot,
                scene: scene,
                selection: selection,
                terrainSample: terrainSample,
                enabledOverlays: [.social, .memory, .weather, .resources]
            )

            drawBackground(in: rect, context: &context, scene: scene)
            drawTerrain(context: &context, scene: scene)
            drawWater(context: &context, scene: scene)
            drawResources(context: &context, scene: scene)
            drawWeather(in: rect, context: &context, scene: scene)
            drawPathTrails(context: &context, overlays: overlays)
            drawSocialLinks(context: &context, overlays: overlays)
            drawMemoryLocations(context: &context, overlays: overlays)
            drawEntities(context: &context, scene: scene)
            drawEventGlyphs(context: &context, overlays: overlays)
            drawTerrainCursor(context: &context, overlays: overlays)
            drawLighting(in: rect, context: &context, scene: scene)
        }
        .overlay(alignment: .topLeading) {
            VStack(alignment: .leading, spacing: 6) {
                Text(String(format: "Cycle %06d", snapshot.cycle))
                    .font(.caption.monospacedDigit())
                Text(phase.title)
                    .font(.headline)
            }
            .padding(12)
            .background(.ultraThinMaterial)
            .clipShape(RoundedRectangle(cornerRadius: 8))
            .padding()
        }
    }

    private func drawBackground(in rect: CGRect, context: inout GraphicsContext, scene: UniverseRenderScene) {
        let top = scene.lighting.phase == .night
            ? Color(red: 0.04, green: 0.07, blue: 0.12)
            : Color(red: 0.09, green: 0.17, blue: 0.18)
        let bottom = scene.lighting.phase == .day
            ? Color(red: 0.24, green: 0.34, blue: 0.30)
            : Color(red: 0.12, green: 0.20, blue: 0.23)

        context.fill(
            Path(rect),
            with: .linearGradient(
                Gradient(colors: [top, bottom]),
                startPoint: .zero,
                endPoint: CGPoint(x: rect.width, y: rect.height)
            )
        )
    }

    private func drawTerrain(context: inout GraphicsContext, scene: UniverseRenderScene) {
        for tile in scene.terrainCache.visibleTiles {
            let screenRect = scene.projection.screenRect(for: tile.mapRect)
            guard screenRect.intersects(CGRect(origin: .zero, size: scene.projection.viewportSize)) else {
                continue
            }

            let sample = tile.samples.sorted { $0.height < $1.height }.dropFirst(tile.samples.count / 2).first ?? tile.samples.first
            let color = sample.map { scene.colorSystem.color(for: $0).color } ?? scene.colorSystem.lowland.color
            context.fill(Path(screenRect), with: .color(color))
            context.stroke(Path(screenRect), with: .color(Color.white.opacity(0.07)), lineWidth: 1)

            let ridgeAlpha = min(0.20, max(0.04, tile.meanHeight / 900.0))
            var path = Path()
            path.move(to: CGPoint(x: screenRect.minX, y: screenRect.midY))
            path.addCurve(
                to: CGPoint(x: screenRect.maxX, y: screenRect.midY + CGFloat(tile.resourceMean.truncatingRemainder(dividingBy: 16.0))),
                control1: CGPoint(x: screenRect.minX + screenRect.width * 0.25, y: screenRect.minY),
                control2: CGPoint(x: screenRect.minX + screenRect.width * 0.72, y: screenRect.maxY)
            )
            context.stroke(path, with: .color(Color.white.opacity(ridgeAlpha)), lineWidth: 1)
        }
    }

    private func drawWater(context: inout GraphicsContext, scene: UniverseRenderScene) {
        for sample in scene.waterSamples where sample.state != .dry {
            let point = scene.projection.screenPoint(for: MapPoint(sample.coordinate))
            let radius: CGFloat = sample.state == .deep ? 15 : 10
            let rect = CGRect(x: point.x - radius, y: point.y - radius, width: radius * 2, height: radius * 2)
            let opacity = sample.state == .shore ? 0.28 : 0.52
            context.fill(Path(ellipseIn: rect), with: .color(scene.colorSystem.shallowWater.color.opacity(opacity)))

            if sample.state == .shore {
                context.stroke(Path(ellipseIn: rect.insetBy(dx: -3, dy: -3)), with: .color(scene.colorSystem.shore.color.opacity(0.55)), lineWidth: 1)
            }
        }
    }

    private func drawResources(context: inout GraphicsContext, scene: UniverseRenderScene) {
        for hint in scene.resources.hints {
            let point = scene.projection.screenPoint(for: MapPoint(hint.coordinate))
            let rect = CGRect(x: point.x - hint.radius, y: point.y - hint.radius, width: hint.radius * 2, height: hint.radius * 2)
            context.fill(Path(ellipseIn: rect), with: .color(hint.color.color))
        }
    }

    private func drawPathTrails(context: inout GraphicsContext, overlays: RenderOverlayScene) {
        for trail in overlays.pathTrails where trail.shouldDraw {
            var path = Path()
            for (index, point) in trail.points.enumerated() {
                if index == 0 {
                    path.move(to: point.screenPosition)
                } else {
                    path.addLine(to: point.screenPosition)
                }
            }
            context.stroke(path, with: .color(trail.color.color.opacity(trail.isSelectedTrail ? 0.64 : 0.32)), lineWidth: trail.isSelectedTrail ? 2 : 1)
        }
    }

    private func drawSocialLinks(context: inout GraphicsContext, overlays: RenderOverlayScene) {
        for link in overlays.socialLinks {
            var path = Path()
            path.move(to: link.sourceScreenPosition)
            path.addLine(to: link.targetScreenPosition)
            context.stroke(path, with: .color(link.color.color), lineWidth: link.lineWidth)
        }
    }

    private func drawMemoryLocations(context: inout GraphicsContext, overlays: RenderOverlayScene) {
        for memory in overlays.memoryLocations.prefix(6) {
            let rect = CGRect(x: memory.screenPosition.x - 5, y: memory.screenPosition.y - 5, width: 10, height: 10)
            context.stroke(Path(ellipseIn: rect), with: .color(memory.color.color), lineWidth: 2)
        }
    }

    private func drawWeather(in rect: CGRect, context: inout GraphicsContext, scene: UniverseRenderScene) {
        if scene.weather.cloudCoverage > 0.18 {
            let cloudCount = Int(scene.weather.cloudCoverage * 6.0) + 1
            for index in 0..<cloudCount {
                let x = rect.width * CGFloat(index + 1) / CGFloat(cloudCount + 1)
                let y = rect.height * (0.12 + CGFloat(index % 3) * 0.045)
                let cloudRect = CGRect(x: x - 54, y: y, width: 108, height: 24)
                context.fill(Path(roundedRect: cloudRect, cornerRadius: 12), with: .color(Color.white.opacity(0.08 + scene.weather.cloudCoverage * 0.12)))
            }
        }

        if scene.weather.rainIntensity > 0 {
            for index in 0..<18 {
                let x = rect.width * CGFloat(index) / 18.0
                let start = CGPoint(x: x, y: rect.height * 0.18 + CGFloat(index % 4) * 26)
                var path = Path()
                path.move(to: start)
                path.addLine(to: CGPoint(x: start.x - CGFloat(scene.weather.windVector.x * 2), y: start.y + 18))
                context.stroke(path, with: .color(Color.cyan.opacity(0.16 + scene.weather.rainIntensity * 0.2)), lineWidth: 1)
            }
        }

        if scene.weather.lightningVisible {
            var bolt = Path()
            bolt.move(to: CGPoint(x: rect.midX, y: 20))
            bolt.addLine(to: CGPoint(x: rect.midX - 18, y: 76))
            bolt.addLine(to: CGPoint(x: rect.midX + 10, y: 70))
            bolt.addLine(to: CGPoint(x: rect.midX - 4, y: 126))
            context.stroke(bolt, with: .color(Color.yellow.opacity(0.72)), lineWidth: 3)
        }
    }

    private func drawEntities(context: inout GraphicsContext, scene: UniverseRenderScene) {
        for marker in scene.apeMarkers {
            let point = marker.screenPosition
            let markerRect = CGRect(
                x: point.x - marker.radius,
                y: point.y - marker.radius,
                width: marker.radius * 2,
                height: marker.radius * 2
            )
            context.fill(Path(ellipseIn: markerRect), with: .color(marker.color.color))

            let heading = CGPoint(
                x: point.x + cos(marker.facingDegrees * .pi / 180.0) * marker.radius * 1.8,
                y: point.y + sin(marker.facingDegrees * .pi / 180.0) * marker.radius * 1.8
            )
            var facing = Path()
            facing.move(to: point)
            facing.addLine(to: heading)
            context.stroke(facing, with: .color(Color.black.opacity(0.42)), lineWidth: 1)

            if marker.isSelected {
                context.stroke(Path(ellipseIn: markerRect.insetBy(dx: -8, dy: -8)), with: .color(scene.colorSystem.markerSelected.color.opacity(0.65)), lineWidth: 2)
            }
        }
    }

    private func drawEventGlyphs(context: inout GraphicsContext, overlays: RenderOverlayScene) {
        for glyph in overlays.eventGlyphs where glyph.isExpired == false {
            let rect = CGRect(
                x: glyph.screenPosition.x - glyph.radius,
                y: glyph.screenPosition.y - glyph.radius,
                width: glyph.radius * 2,
                height: glyph.radius * 2
            )
            context.fill(Path(roundedRect: rect, cornerRadius: 4), with: .color(glyph.color.color.opacity(0.72)))
            context.stroke(Path(roundedRect: rect.insetBy(dx: -2, dy: -2), cornerRadius: 6), with: .color(Color.black.opacity(0.22)), lineWidth: 1)
        }
    }

    private func drawTerrainCursor(context: inout GraphicsContext, overlays: RenderOverlayScene) {
        guard overlays.cursor.isVisible else {
            return
        }

        let point = overlays.cursor.screenPosition
        var horizontal = Path()
        horizontal.move(to: CGPoint(x: point.x - 12, y: point.y))
        horizontal.addLine(to: CGPoint(x: point.x + 12, y: point.y))
        var vertical = Path()
        vertical.move(to: CGPoint(x: point.x, y: point.y - 12))
        vertical.addLine(to: CGPoint(x: point.x, y: point.y + 12))
        context.stroke(horizontal, with: .color(Color.white.opacity(0.72)), lineWidth: 1)
        context.stroke(vertical, with: .color(Color.white.opacity(0.72)), lineWidth: 1)
    }

    private func drawLighting(in rect: CGRect, context: inout GraphicsContext, scene: UniverseRenderScene) {
        guard scene.lighting.overlayOpacity > 0 else {
            return
        }

        context.fill(Path(rect), with: .color(scene.lighting.tint.color.opacity(scene.lighting.overlayOpacity)))
    }
}

private struct InspectorPanel: View {
    let snapshot: UniverseSnapshot
    let metadata: RunMetadata?
    let health: SimulationHealthSnapshot

    var body: some View {
        ScrollView {
            VStack(alignment: .leading, spacing: 20) {
                SectionBlock(title: "Run") {
                    MetricRow(label: "Scenario", value: metadata?.scenarioName ?? "--")
                    MetricRow(label: "Seed", value: metadata.map { "0x" + String($0.seed, radix: 16, uppercase: true) } ?? "--")
                    MetricRow(label: "Mode", value: metadata?.mode.rawValue ?? "--")
                    MetricRow(label: "Interventions", value: "\(metadata?.interventionCount ?? 0)")
                }

                SectionBlock(title: "World") {
                    MetricRow(label: "Population", value: "\(snapshot.population.count)/\(snapshot.population.max)")
                    MetricRow(label: "Land Date", value: "\(snapshot.landDate)")
                    MetricRow(label: "Land Time", value: "\(snapshot.landTime)")
                    MetricRow(label: "Weather", value: snapshot.weather.condition.map(String.init) ?? "--")
                }

                SectionBlock(title: "Health") {
                    MetricRow(label: "Cycle Latency", value: String(format: "%.2f ms", health.cycleLatencyMilliseconds))
                    MetricRow(label: "Bridge Errors", value: "\(health.bridgeErrorCount)")
                    MetricRow(label: "Dropped Snapshots", value: "\(health.droppedSnapshotCount)")
                    MetricRow(label: "State", value: health.isHealthy ? "Nominal" : "Review")
                }

                SectionBlock(title: "Run Log") {
                    Text("Runtime events and intervention evidence will be attached to run metadata.")
                        .foregroundStyle(.secondary)
                }
            }
            .padding(20)
        }
        .background(Color(uiColor: .secondarySystemBackground))
    }
}

private struct SectionBlock<Content: View>: View {
    let title: String
    @ViewBuilder let content: Content

    var body: some View {
        VStack(alignment: .leading, spacing: 10) {
            Text(title)
                .font(.headline)
            content
                .font(.callout)
        }
    }
}

private struct MetricRow: View {
    let label: String
    let value: String

    var body: some View {
        HStack {
            Text(label)
                .foregroundStyle(.secondary)
            Spacer()
            Text(value)
                .monospacedDigit()
        }
    }
}

#Preview {
    ContentView()
}
