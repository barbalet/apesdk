import SwiftUI

struct ContentView: View {
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
                WorldCanvas()

                Divider()

                InspectorPanel()
                    .frame(width: 340)
            }
            .toolbar {
                ToolbarItemGroup(placement: .primaryAction) {
                    Button {
                    } label: {
                        Label("Pause", systemImage: "pause.fill")
                    }

                    Button {
                    } label: {
                        Label("Step", systemImage: "forward.frame.fill")
                    }

                    Button {
                    } label: {
                        Label("Run Log", systemImage: "list.bullet.rectangle")
                    }
                }
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
    var body: some View {
        Canvas { context, size in
            let rect = CGRect(origin: .zero, size: size)
            context.fill(Path(rect), with: .linearGradient(
                Gradient(colors: [
                    Color(red: 0.06, green: 0.13, blue: 0.16),
                    Color(red: 0.14, green: 0.27, blue: 0.22),
                    Color(red: 0.19, green: 0.32, blue: 0.40)
                ]),
                startPoint: .zero,
                endPoint: CGPoint(x: size.width, y: size.height)
            ))

            drawTerrain(in: rect, context: &context)
            drawWater(in: rect, context: &context)
            drawEntities(in: rect, context: &context)
        }
        .overlay(alignment: .topLeading) {
            VStack(alignment: .leading, spacing: 6) {
                Text("Cycle 000000")
                    .font(.caption.monospacedDigit())
                Text("Living world viewport")
                    .font(.headline)
            }
            .padding(12)
            .background(.ultraThinMaterial)
            .clipShape(RoundedRectangle(cornerRadius: 8))
            .padding()
        }
    }

    private func drawTerrain(in rect: CGRect, context: inout GraphicsContext) {
        for index in 0..<18 {
            let y = rect.height * CGFloat(index) / 18.0
            var path = Path()
            path.move(to: CGPoint(x: 0, y: y + CGFloat(index % 3) * 8))
            path.addCurve(
                to: CGPoint(x: rect.width, y: y + 32),
                control1: CGPoint(x: rect.width * 0.28, y: y - 28),
                control2: CGPoint(x: rect.width * 0.68, y: y + 54)
            )
            context.stroke(path, with: .color(Color.white.opacity(0.08)), lineWidth: 1)
        }
    }

    private func drawWater(in rect: CGRect, context: inout GraphicsContext) {
        var path = Path()
        path.move(to: CGPoint(x: 0, y: rect.height * 0.68))
        path.addCurve(
            to: CGPoint(x: rect.width, y: rect.height * 0.62),
            control1: CGPoint(x: rect.width * 0.32, y: rect.height * 0.58),
            control2: CGPoint(x: rect.width * 0.58, y: rect.height * 0.74)
        )
        path.addLine(to: CGPoint(x: rect.width, y: rect.height))
        path.addLine(to: CGPoint(x: 0, y: rect.height))
        path.closeSubpath()
        context.fill(path, with: .color(Color(red: 0.14, green: 0.37, blue: 0.48).opacity(0.72)))
    }

    private func drawEntities(in rect: CGRect, context: inout GraphicsContext) {
        let points = [
            CGPoint(x: rect.width * 0.26, y: rect.height * 0.42),
            CGPoint(x: rect.width * 0.41, y: rect.height * 0.53),
            CGPoint(x: rect.width * 0.63, y: rect.height * 0.35),
            CGPoint(x: rect.width * 0.76, y: rect.height * 0.59),
            CGPoint(x: rect.width * 0.54, y: rect.height * 0.69)
        ]

        for (index, point) in points.enumerated() {
            let marker = CGRect(x: point.x - 6, y: point.y - 6, width: 12, height: 12)
            context.fill(Path(ellipseIn: marker), with: .color(index == 0 ? .yellow : .white.opacity(0.82)))
            context.stroke(Path(ellipseIn: marker.insetBy(dx: -7, dy: -7)), with: .color(.white.opacity(0.18)), lineWidth: 1)
        }
    }
}

private struct InspectorPanel: View {
    var body: some View {
        ScrollView {
            VStack(alignment: .leading, spacing: 20) {
                SectionBlock(title: "Selected Ape") {
                    MetricRow(label: "Name", value: "Unselected")
                    MetricRow(label: "Energy", value: "--")
                    MetricRow(label: "Drive", value: "--")
                    MetricRow(label: "State", value: "Observing")
                }

                SectionBlock(title: "Memory") {
                    Text("Episodic events will show time, place, affect, involved beings, and event arguments.")
                        .foregroundStyle(.secondary)
                }

                SectionBlock(title: "Social Graph") {
                    Text("Relationships will connect summary values back to memories and recent encounters.")
                        .foregroundStyle(.secondary)
                }

                SectionBlock(title: "Run Log") {
                    Text("Simulation-changing interventions will be marked and preserved with run metadata.")
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
