#ifndef BRONZE_WORKSPACE_RENDER_H
#define BRONZE_WORKSPACE_RENDER_H

#include <math.h>
#include <stdlib.h>

/* The workspace renders the native heightfield and registry in map units.
 * Detail is derived from world coordinates so it stays put in both cameras. */
typedef struct {
    int x, y, radius;
    bronze_site_kind kind;
    uint64_t id;
} BronzeRenderSite;

typedef struct {
    BronzeRenderSite sites[BRONZE_SITE_LIMIT];
    unsigned count;
    int tide;
    const n_byte *heights;
    const BronzeSettlementLayout *settlement;
} BronzeRenderLand;

typedef struct {
    double height;
    int r, g, b;
} BronzeRenderSample;

static unsigned bronze_render_noise(int x, int y)
{
    uint32_t value = (uint32_t)x * UINT32_C(374761393) + (uint32_t)y * UINT32_C(668265263);
    value = (value ^ (value >> 13)) * UINT32_C(1274126177);
    return value ^ (value >> 16);
}

static int bronze_render_wrap(int value)
{
    return value & (MAP_DIMENSION - 1);
}

static int bronze_render_delta(int a, int b)
{
    return bronze_render_wrap(a - b + MAP_DIMENSION / 2) - MAP_DIMENSION / 2;
}

static n_byte bronze_render_channel(int value)
{
    return (n_byte)(value < 0 ? 0 : value > 255 ? 255 : value);
}

static void bronze_render_pixel(n_byte4 *pixel, int r, int g, int b)
{
    n_byte *bytes = (n_byte *)pixel;
    bytes[0] = bronze_render_channel(r);
    bytes[1] = bronze_render_channel(g);
    bytes[2] = bronze_render_channel(b);
    bytes[3] = 255;
}

static double bronze_render_bilinear(double southwest, double southeast,
                                      double northwest, double northeast,
                                      double u, double v)
{
    return (1 - v) * (southwest + u * (southeast - southwest))
           + v * (northwest + u * (northeast - northwest));
}

enum { BRONZE_RENDER_BEACH_HEIGHT = 8 };

/* Half-height relief for Bronze presentation; native simulation heights stay intact. */
static const double bronze_render_height_scale = 0.155;

static int bronze_render_tree_ground(const BronzeRenderLand *land, int x, int y)
{
    int wx = bronze_render_wrap(x), wy = bronze_render_wrap(y);
    int east = bronze_render_wrap(wx + 1), north = bronze_render_wrap(wy + 1);
    int dry_height = (land->tide > TIDE_MAX ? land->tide : TIDE_MAX) + BRONZE_RENDER_BEACH_HEIGHT;
    if (land->settlement && (land->settlement->building_at[wy*MAP_DIMENSION+wx] ||
                             land->settlement->track[wy*MAP_DIMENSION+wx])) return 0;
    /* A tree's complete cell footprint must clear the beach and water.
     * Use native ground heights before water clamping or site decoration. */
    return land->heights[wy * MAP_DIMENSION + wx] > dry_height
           && land->heights[wy * MAP_DIMENSION + east] > dry_height
           && land->heights[north * MAP_DIMENSION + wx] > dry_height
           && land->heights[north * MAP_DIMENSION + east] > dry_height;
}

static BronzeRenderSample bronze_render_ground_vertex(const BronzeRenderLand *land, int x, int y)
{
    int wx = bronze_render_wrap(x), wy = bronze_render_wrap(y);
    double z = land->heights[wy * MAP_DIMENSION + wx];
    double slope_x = (land->heights[wy * MAP_DIMENSION + bronze_render_wrap(wx + 1)]
                      - land->heights[wy * MAP_DIMENSION + bronze_render_wrap(wx - 1)]) * 0.5;
    double slope_y = (land->heights[bronze_render_wrap(wy + 1) * MAP_DIMENSION + wx]
                      - land->heights[bronze_render_wrap(wy - 1) * MAP_DIMENSION + wx]) * 0.5;
    double slope = -1.5 * slope_x - 0.9 * slope_y;
    int light = (int)fmax(-32, fmin(32, slope));
    unsigned noise = bronze_render_noise(wx, wy);
    int green = (int)(noise % 19);
    int meadow = (int)(11 * sin(wx * 0.19 + sin(wy * 0.12)) + 7 * sin(wy * 0.37 - wx * 0.13));
    int relief = (int)fmin(18, fmax(0, (z - WATER_MAP) * 0.32));
    BronzeRenderSample sample = {z * bronze_render_height_scale, 94 + light - green / 2 + meadow + relief,
                                 105 + light + green / 3 + meadow / 2, 48 + light + relief};
    if (fabs(slope) > 38) {
        sample.r += 22; sample.g += 15; sample.b += 13;
    }
    return sample;
}

/* Both cameras sample the authoritative spatial records in native map cells. */
static int bronze_render_settlement(const BronzeRenderLand *land,double x,double y,
                                     BronzeRenderSample *sample, int overhead)
{
    const BronzeSettlementLayout *layout=land->settlement;
    if (!layout) return 0;
    int ix=(int)floor(x),iy=(int)floor(y);
    unsigned cell=(unsigned)(bronze_render_wrap(iy)*MAP_DIMENSION+bronze_render_wrap(ix));
    unsigned index=layout->building_at[cell];
    double u=x-floor(x),v=y-floor(y);
    int grain=(int)(bronze_render_noise((int)floor(x*19),(int)floor(y*19))%11)-5;
    if (index && index<=layout->building_count) {
        const BronzeBuilding *building=&layout->buildings[index-1];
        double a=bronze_render_delta(bronze_render_wrap(ix),(int)building->cells[0].x)+u;
        double b=bronze_render_delta(bronze_render_wrap(iy),(int)building->cells[0].y)+v;
        double angle=bronze_building_bearing(building)*0.017453292519943295;
        double local_x=cos(angle)*(a-1)+sin(angle)*(b-1);
        double local_y=-sin(angle)*(a-1)+cos(angle)*(b-1);
        double clearing=building->kind==BRONZE_BUILDING_PLAZA?1.45:0.98;
        double center=building->kind==BRONZE_BUILDING_PLAZA?1.5:1;
        if (hypot(a-center,b-center)>clearing) return 0;
        sample->r=155+grain; sample->g=127+grain; sample->b=87+grain;
        if (building->kind==BRONZE_BUILDING_HOME) {
            double radius=hypot(a-1,b-1);
            if (radius<0.94) {
                int shade=(int)((a+b-2)*19)+grain;
                if (overhead) sample->height+=1.0+(0.94-radius)*1.8;
                sample->r=163-shade; sample->g=124-shade; sample->b=66-shade/2;
            }
        } else if (building->kind==BRONZE_BUILDING_STORE) {
            if (fabs(local_x)<0.65 && fabs(local_y)<0.7) {
                double ridge=1-fabs(local_x)/0.65;
                if (overhead) sample->height+=1.1+ridge*0.75;
                int shade=(int)((local_x+1)*17)+grain+(int)(5*sin(local_y*34));
                sample->r=171-shade; sample->g=131-shade; sample->b=75-shade/2;
            }
        } else if (building->kind==BRONZE_BUILDING_WORK_YARD) {
            a=local_x+1; b=local_y+1;
            double radius=hypot(a-1,b-1);
            if (building->profession==BRONZE_PREMISE_METAL && radius<0.37) {
                if (overhead) sample->height+=0.38;
                sample->r=radius<0.16?193:63; sample->g=radius<0.16?91:58; sample->b=39;
            } else if (building->profession==BRONZE_PREMISE_POTTERY &&
                       hypot(fmod(a+0.1,0.65)-0.3,fmod(b+0.1,0.65)-0.3)<0.17) {
                if (overhead) sample->height+=0.35; sample->r=171+grain; sample->g=91+grain; sample->b=51;
            } else if (building->profession==BRONZE_PREMISE_WOOD && fabs(b-1)<0.3) {
                if (overhead) sample->height+=0.38; sample->r=110+grain; sample->g=73+grain; sample->b=40;
            } else if (building->profession==BRONZE_PREMISE_FOOD && radius<0.48) {
                if (overhead) sample->height+=0.65*(1-radius/0.6); sample->r=132+grain; sample->g=83+grain; sample->b=52;
            }
        }
        return 1;
    }
    unsigned door=layout->entrance_at[cell];
    if (door && door<=layout->building_count && layout->solid[
        bronze_render_wrap((int)layout->buildings[door-1].cells[0].y)*MAP_DIMENSION+
        bronze_render_wrap((int)layout->buildings[door-1].cells[0].x)]) {
        sample->r=116+grain; sample->g=89+grain; sample->b=55+grain;
        if (fabs(u-0.5)>0.33 && fabs(v-0.5)<0.16) {
            if (overhead) sample->height+=0.65; sample->r=90; sample->g=63; sample->b=36;
        }
        return 1;
    }
    if (layout->track[cell]) {
        unsigned links=layout->track_links[cell];
        double half=layout->track[cell]==2?0.44:0.23;
        int drawn=hypot(u-0.5,v-0.5)<half;
        /* Round two-way bends through a quadratic centerline instead of
         * painting an L-shaped right angle into every corner cell. */
        if (links==3 || links==6 || links==12 || links==9) {
            static const double ex[]={1,0.5,0,0.5},ey[]={0.5,1,0.5,0};
            int first=-1,second=-1;
            for (int d=0;d<4;++d) if (links&(1u<<d)) {
                if (first<0) first=d; else second=d;
            }
            drawn=0;
            for (int step=0;step<=12;++step) {
                double t=step/12.0,one=1-t;
                double px=one*one*ex[first]+one*t+t*t*ex[second];
                double py=one*one*ey[first]+one*t+t*t*ey[second];
                if (hypot(u-px,v-py)<half) { drawn=1;break; }
            }
        } else {
            if ((links&1) && u>=0.5 && fabs(v-0.5)<half) drawn=1;
            if ((links&2) && v>=0.5 && fabs(u-0.5)<half) drawn=1;
            if ((links&4) && u<=0.5 && fabs(v-0.5)<half) drawn=1;
            if ((links&8) && v<=0.5 && fabs(u-0.5)<half) drawn=1;
        }
        if (drawn) { sample->r=153+grain; sample->g=126+grain; sample->b=86+grain; }
        return 1;
    }
    unsigned shade_cell=(unsigned)(bronze_render_wrap((int)floor(y-0.9))*MAP_DIMENSION+
                                    bronze_render_wrap((int)floor(x-1.4)));
    unsigned caster=layout->building_at[shade_cell];
    if (caster && layout->solid[shade_cell]) {
        const BronzeBuilding *building=&layout->buildings[caster-1];
        double sx=bronze_render_delta(bronze_render_wrap((int)floor(x-1.4)),building->cells[0].x)+
                  (x-1.4-floor(x-1.4))-1;
        double sy=bronze_render_delta(bronze_render_wrap((int)floor(y-0.9)),building->cells[0].y)+
                  (y-0.9-floor(y-0.9))-1;
        double shade=1-0.2*fmax(0,1-hypot(sx,sy));
        sample->r=(int)(sample->r*shade); sample->g=(int)(sample->g*shade); sample->b=(int)(sample->b*shade);
    }
    return 0;
}

static BronzeRenderSample bronze_render_sample(const BronzeRenderLand *land, double x, double y, int canopy)
{
    int ix = (int)floor(x), iy = (int)floor(y);
    int wx = bronze_render_wrap(ix), wy = bronze_render_wrap(iy);
    double u = x - ix, v = y - iy;
    BronzeRenderSample southwest = bronze_render_ground_vertex(land, wx, wy);
    BronzeRenderSample southeast = bronze_render_ground_vertex(land, wx + 1, wy);
    BronzeRenderSample northwest = bronze_render_ground_vertex(land, wx, wy + 1);
    BronzeRenderSample northeast = bronze_render_ground_vertex(land, wx + 1, wy + 1);
    /* Shared corner colors include vertex lighting. Both height and material
     * meet at the same values on either side of every wrapped cell edge. */
    BronzeRenderSample sample = {
        bronze_render_bilinear(southwest.height, southeast.height, northwest.height, northeast.height, u, v),
        (int)lround(bronze_render_bilinear(southwest.r, southeast.r, northwest.r, northeast.r, u, v)),
        (int)lround(bronze_render_bilinear(southwest.g, southeast.g, northwest.g, northeast.g, u, v)),
        (int)lround(bronze_render_bilinear(southwest.b, southeast.b, northwest.b, northeast.b, u, v))
    };
    /* The seabed and beach use a fixed datum. Only the water surface follows
     * the current tide, so exposed or submerged sand keeps its material. */
    double elevation = sample.height / bronze_render_height_scale - TIDE_MAX;
    if (elevation <= BRONZE_RENDER_BEACH_HEIGHT) {
        double grass = fmax(0, (elevation - 3) / (BRONZE_RENDER_BEACH_HEIGHT - 3));
        grass = grass * grass * (3 - 2 * grass);
        int light = (sample.r + sample.g + sample.b) / 12 - 20;
        int sand_r = 183 + light;
        int sand_g = 163 + light;
        int sand_b = 120 + light;
        sample.r = (int)lround(sand_r * (1 - grass) + sample.r * grass);
        sample.g = (int)lround(sand_g * (1 - grass) + sample.g * grass);
        sample.b = (int)lround(sand_b * (1 - grass) + sample.b * grass);
        bronze_render_settlement(land,x,y,&sample,canopy);
        return sample;
    }
    if (bronze_render_settlement(land,x,y,&sample,canopy)) return sample;
    unsigned noise = bronze_render_noise(wx, wy);
    int grain = (int)(bronze_render_noise((int)floor(x * 8), (int)floor(y * 8)) % 13) - 6;
    int green = (int)(noise % 19);
    int light = (sample.r + sample.g + sample.b) / 3 - 82;
    for (unsigned i = 0; i < land->count; ++i) {
        const BronzeRenderSite *site = &land->sites[i];
        int dx = bronze_render_delta(wx, site->x), dy = bronze_render_delta(wy, site->y);
        if (abs(dx) > site->radius || abs(dy) > site->radius) continue;
        if (site->kind == BRONZE_FIELD) {
            double a = dx + u, b = dy + v;
            if (fabs(a) > site->radius - 0.25 || fabs(b) > site->radius - 0.35) break;
            int furrow = (int)(11 * cos((a + b * 0.14) * 18));
            int headland = fabs(b) > site->radius - 0.8 ? -12 : 0;
            return (BronzeRenderSample){sample.height, 131 + furrow + light + headland,
                                       103 + furrow + light + headland, 52 + furrow / 2 + light};
        }
        if (site->kind == BRONZE_TOWN) continue;
        if (site->kind == BRONZE_WOODLAND) {
            green += 30;
            break;
        }
        double a = dx + u, b = dy + v;
        double distance = a * a + b * b;
        if (distance < 1.1)
            return (BronzeRenderSample){sample.height, 49 + light, 44 + light, 39 + light};
        if (distance < site->radius * site->radius)
            return (BronzeRenderSample){sample.height, 118 + light + grain,
                                       105 + light, 87 + light - grain};
        break;
    }
    if (canopy && bronze_render_tree_ground(land, wx, wy) && (int)(noise % 100) < green + 8) {
        double cx = u - 0.5, cy = v - 0.5;
        double radius = cx * cx + cy * cy;
        double edge = 0.15 + grain * 0.005;
        if (radius < edge) {
            int shade = light + (int)((cx + cy) * 31) + grain;
            return (BronzeRenderSample){sample.height + 1.6 + (edge - radius) * 7,
                                       48 + shade, 70 + shade, 30 + shade};
        }
        if ((cx - 0.12) * (cx - 0.12) + (cy - 0.12) * (cy - 0.12) < edge) {
            sample.r = sample.r * 3 / 4;
            sample.g = sample.g * 3 / 4;
            sample.b = sample.b * 3 / 4;
        }
    }
    return sample;
}

typedef struct {
    double x, y, fx, fy, eye;
} BronzeRenderCamera;

static BronzeRenderCamera bronze_render_camera_distance(double ax, double ay, double bearing,
                                               const BronzeRenderLand *land, double distance, double lift)
{
    double fx = cos(bearing), fy = sin(bearing);
    double x = ax - fx * distance, y = ay - fy * distance;
    double eye = fmax(land->tide * bronze_render_height_scale,
                      fmax(bronze_render_sample(land, x, y, 0).height,
                           bronze_render_sample(land, ax, ay, 0).height)) + lift;
    return (BronzeRenderCamera){x, y, fx, fy, eye};
}

static BronzeRenderCamera bronze_render_camera(double ax,double ay,double bearing,
                                               const BronzeRenderLand *land)
{
    return bronze_render_camera_distance(ax,ay,bearing,land,12,4.2);
}

static void bronze_render_marker(n_byte4 *pixels, int width, int height, int x, int y,
                                int radius, int r, int g, int b, int ring)
{
    if (x < -radius || x >= width + radius || y < -radius || y >= height + radius) return;
    for (int dy = -radius; dy <= radius; ++dy) {
        if (y + dy < 0 || y + dy >= height) continue;
        for (int dx = -radius; dx <= radius; ++dx) {
            if (x + dx < 0 || x + dx >= width) continue;
            int squared = dx * dx + dy * dy;
            if (squared > radius * radius || (ring && squared < (radius - 2) * (radius - 2))) continue;
            bronze_render_pixel(pixels + (y + dy) * width + x + dx, r, g, b);
        }
    }
}

static BronzeRenderSample bronze_render_water(BronzeRenderSample bottom, double water_height,
                                               double x, double y)
{
    double depth = water_height - bottom.height;
    if (depth <= 0) return bottom;
    /* Blend a visible dark blue surface over the seabed. Keep at least
     * half the bottom contribution, with a soft fade at the shoreline. */
    double opacity = 0.45 * fmin(1, depth / 1.2) + 0.05 * (1 - exp(-depth * 0.1));
    double transmission = 1 - opacity;
    double ripple = (sin(x * 1.7 + sin(y * 0.41)) + sin(y * 1.3 - x * 0.25)) * 2;
    double shade = 1 - 0.08 * fmin(1, depth / 0.8);
    bottom.r = (int)lround(bottom.r * shade * transmission + (5 + ripple) * opacity);
    bottom.g = (int)lround(bottom.g * shade * transmission + (33 + ripple) * opacity);
    bottom.b = (int)lround(bottom.b * shade * transmission + (74 + ripple) * opacity);
    return bottom;
}

static void bronze_render_map(n_byte4 *pixels, int width, int height, const BronzeRenderLand *land,
                             simulated_group *group, int cx, int cy, double span, uint64_t selected_site)
{
    double units = span / width;
    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            BronzeRenderSample p = bronze_render_sample(land, cx + (x - width / 2) * units,
                                                        cy + (y - height / 2) * units, 1);
            p = bronze_render_water(p, land->tide * bronze_render_height_scale, cx + (x - width / 2) * units,
                                    cy + (y - height / 2) * units);
            bronze_render_pixel(pixels + y * width + x, p.r, p.g, p.b);
        }
    }
    static const int colors[][3] = {{225, 174, 111}, {214, 198, 121}, {217, 137, 86},
                                    {200, 204, 210}, {142, 183, 105}};
    for (unsigned i = 0; i < land->count; ++i) {
        const BronzeRenderSite *site = &land->sites[i];
        int x = width / 2 + (int)(bronze_render_delta(site->x, cx) / units);
        int y = height / 2 + (int)(bronze_render_delta(site->y, cy) / units);
        const int *color = colors[site->kind];
        if (site->id == selected_site)
            bronze_render_marker(pixels, width, height, x, y, 9, 247, 183, 99, 1);
        bronze_render_marker(pixels, width, height, x, y, 5, 30, 27, 23, 0);
        bronze_render_marker(pixels, width, height, x, y, 5, color[0], color[1], color[2], 1);
        bronze_render_marker(pixels, width, height, x, y, 2, color[0], color[1], color[2], 0);
    }
    if ((selected_site>>63) && land->settlement) {
        for (unsigned b=0;b<land->settlement->building_count;++b) {
            const BronzeBuilding *building=&land->settlement->buildings[b];
            if (building->id!=(selected_site & ~(UINT64_C(1)<<63))) continue;
            int x=width/2+(int)((bronze_render_delta((int)building->entrance.x,cx)+0.5)/units);
            int y=height/2+(int)((bronze_render_delta((int)building->entrance.y,cy)+0.5)/units);
            bronze_render_marker(pixels,width,height,x,y,8,247,183,99,1);
        }
    }
    for (n_uint i = 0; i < group->num; ++i) {
        simulated_being *being = &group->beings[i];
        int bx = (int)APESPACE_TO_MAPSPACE(being_location_x(being));
        int by = (int)APESPACE_TO_MAPSPACE(being_location_y(being));
        int x = width / 2 + (int)(bronze_render_delta(bx, cx) / units);
        int y = height / 2 + (int)(bronze_render_delta(by, cy) / units);
        int selected = being == group->select;
        bronze_render_marker(pixels, width, height, x, y, selected ? 7 : 2,
                             248, selected ? 188 : 219, 111, selected);
    }
}

typedef struct {
    n_byte4 *pixels;
    float *depths;
    int width, height;
} BronzeRenderFrame;

static void bronze_depth_pixel(BronzeRenderFrame *frame, int x, int y, float depth,
                              int r, int g, int b)
{
    if (x < 0 || y < 0 || x >= frame->width || y >= frame->height) return;
    int index = y * frame->width + x;
    if (frame->depths[index] != 0 && depth > frame->depths[index]) return;
    frame->depths[index] = depth;
    bronze_render_pixel(frame->pixels + index, r, g, b);
}

static void bronze_tree_branch(BronzeRenderFrame *frame, int x, int y, int dx, int dy,
                               float depth, int thickness)
{
    int length = abs(dx) + abs(dy);
    if (!length) return;
    for (int i = 0; i <= length; ++i) {
        int bx = x + dx * i / length, by = y + dy * i / length;
        for (int offset = -thickness; offset <= thickness; ++offset)
            bronze_depth_pixel(frame, bx + offset, by, depth, 84, 63, 41);
    }
}

static void bronze_tree_crown(BronzeRenderFrame *frame, int x, int y, int radius,
                              float depth, unsigned seed)
{
    if (radius < 1) radius = 1;
    for (int dy = -radius; dy <= radius; ++dy) {
        for (int dx = -radius; dx <= radius; ++dx) {
            unsigned leaf = bronze_render_noise(dx + (int)(seed & 255), dy);
            int shape = dx * dx + dy * dy * 5 / 4;
            int edge = radius * radius * (82 + (int)(leaf % 29)) / 100;
            if (shape > edge || (leaf % 9 == 0 && shape > edge / 3)) continue;
            int light = (int)(leaf % 17) + (dy - dx) * 12 / radius;
            int haze = (int)(depth * 0.65);
            bronze_depth_pixel(frame, x + dx, y + dy, depth,
                              48 + light + haze, 77 + light + haze, 35 + light / 2 + haze);
        }
    }
}

static int bronze_tree_allowed(const BronzeRenderLand *land, int x, int y, unsigned seed)
{
    if (!bronze_render_tree_ground(land, x, y)) return 0;
    int density = (int)(seed % 19) + 8;
    for (unsigned i = 0; i < land->count; ++i) {
        const BronzeRenderSite *site = &land->sites[i];
        if (abs(bronze_render_delta(x, site->x)) > site->radius ||
            abs(bronze_render_delta(y, site->y)) > site->radius) continue;
        if (site->kind != BRONZE_WOODLAND) return 0;
        density += 30;
        break;
    }
    return (int)(seed % 100) < density;
}

static void bronze_render_trees(BronzeRenderFrame *frame, const BronzeRenderLand *land,
                                BronzeRenderCamera camera, double focal, double horizon)
{
    enum { tree_range = 55 };
    int camera_x = (int)floor(camera.x), camera_y = (int)floor(camera.y);
    for (int y = camera_y - tree_range; y <= camera_y + tree_range; ++y) {
        for (int x = camera_x - tree_range; x <= camera_x + tree_range; ++x) {
            double dx = x + 0.5 - camera.x, dy = y + 0.5 - camera.y;
            double depth = dx * camera.fx + dy * camera.fy;
            if (depth < 1.8 || depth > (double)tree_range) continue;
            double side = -dx * camera.fy + dy * camera.fx;
            double sx = frame->width * 0.5 + side * focal / depth;
            if (sx < -50 || sx >= frame->width + 50) continue;
            unsigned seed = bronze_render_noise(bronze_render_wrap(x), bronze_render_wrap(y));
            if (!bronze_tree_allowed(land, x, y, seed)) continue;
            BronzeRenderSample ground = bronze_render_sample(land, x + 0.5, y + 0.5, 0);
            int base = (int)(horizon + (ground.height - camera.eye) * focal / depth);
            int size = (int)((1.5 + (seed % 13) * 0.075) * focal / depth);
            if (size < 4 || base > frame->height || base + size * 2 < 0) continue;
            int px = (int)sx;
            int lean = (seed & 128) ? -1 : 1;
            int branch = size / 4;
            int thickness = size / 26;
            float tree_depth = (float)(depth - 0.25);
            bronze_tree_branch(frame, px, base, lean * thickness, size, tree_depth, thickness);
            bronze_tree_branch(frame, px, base + size / 3, -branch, size / 3, tree_depth, thickness / 2);
            bronze_tree_branch(frame, px, base + size / 2, branch, size / 4, tree_depth, thickness / 2);
            bronze_tree_crown(frame, px - branch, base + size * 2 / 3, size / 5, tree_depth, seed);
            bronze_tree_crown(frame, px + branch, base + size * 3 / 4, size / 5, tree_depth, seed >> 8);
            bronze_tree_crown(frame, px + lean * thickness, base + size, size / 4, tree_depth, seed >> 16);
        }
    }
}

#include "bronze_workspace_objects.h"

static void bronze_render_terrain(n_byte4 *pixels, int width, int height, const BronzeRenderLand *land,
                                 simulated_group *group, BronzeRenderCamera camera)
{
    double focal = width * 0.72, horizon = height * 0.61;
    BronzeRenderFrame frame = {pixels, calloc((n_uint)width * height, sizeof(float)), width, height};
    for (int x = 0; x < width; ++x) {
        double right = (x - width * 0.5) / focal;
        double rx = camera.fx - camera.fy * right, ry = camera.fy + camera.fx * right;
        int ceiling = 0;
        double previous_top = 0, previous_depth = 1.1;
        BronzeRenderSample previous = bronze_render_sample(land, camera.x + rx * 1.1,
                                                             camera.y + ry * 1.1, 0);
        for (double depth = 1.1; depth < 118 && ceiling < height; depth += 0.19 + depth * 0.019) {
            BronzeRenderSample p = bronze_render_sample(land, camera.x + rx * depth, camera.y + ry * depth, 0);
            double top = horizon + (p.height - camera.eye) * focal / depth;
            /* Intersect each visible row with the projected segment. A single
             * color and depth per span turns every nearby slope into a stripe. */
            for (; ceiling < height && ceiling + 0.5 < top; ++ceiling) {
                double mix = top > previous_top ? (ceiling + 0.5 - previous_top) / (top - previous_top) : 1;
                mix = fmax(0, fmin(1, mix));
                double hit = previous_depth + (depth - previous_depth) * mix;
                double haze = fmin(0.76, hit / 150);
                double detail_x = camera.x + rx * hit, detail_y = camera.y + ry * hit;
                int texture = (int)(bronze_render_noise((int)floor(detail_x * 17),
                                                      (int)floor(detail_y * 17)) % 11) - 5;
                int r = (int)((previous.r + (p.r - previous.r) * mix + texture) * (1 - haze) + 158 * haze);
                int g = (int)((previous.g + (p.g - previous.g) * mix + texture) * (1 - haze) + 172 * haze);
                int b = (int)((previous.b + (p.b - previous.b) * mix + texture) * (1 - haze) + 166 * haze);
                int index = ceiling * width + x;
                bronze_render_pixel(pixels + index, r, g, b);
                if (frame.depths) frame.depths[index] = (float)hit;
            }
            previous = p;
            previous_top = top;
            previous_depth = depth;
        }
        for (; ceiling < height; ++ceiling) {
            double blue = fmax(0, (ceiling - horizon) / (height - horizon));
            bronze_render_pixel(pixels + ceiling * width + x, (int)(200 - blue * 73),
                                 (int)(211 - blue * 39), (int)(218 - blue * 9));
        }
    }
    if (!frame.depths) return;
    double water_height = land->tide * bronze_render_height_scale;
    for (int y = 0; y < height && y + 0.5 < horizon; ++y) {
        double depth = (camera.eye - water_height) * focal / (horizon - y - 0.5);
        if (depth < 1.1 || depth >= 118) continue;
        for (int x = 0; x < width; ++x) {
            int index = y * width + x;
            if (frame.depths[index] != 0 && frame.depths[index] <= depth) continue;
            double right = (x - width * 0.5) / focal;
            double wx = camera.x + (camera.fx - camera.fy * right) * depth;
            double wy = camera.y + (camera.fy + camera.fx * right) * depth;
            BronzeRenderSample bed = bronze_render_sample(land, wx, wy, 0);
            if (bed.height >= water_height) continue;
            /* The ground pass supplies the visible bottom through the water.
             * The plane supplies its independent surface depth for occlusion. */
            const n_byte *rgba = (const n_byte *)(pixels + index);
            /* Absorption follows the bottom actually visible along this ray,
             * not the unrelated ground directly beneath the surface crossing. */
            double bottom_depth = frame.depths[index];
            double bottom_height = bottom_depth > 0
                ? bronze_render_sample(land,
                    camera.x + (camera.fx - camera.fy * right) * bottom_depth,
                    camera.y + (camera.fy + camera.fx * right) * bottom_depth, 0).height
                : bed.height;
            BronzeRenderSample under = {bottom_height, rgba[0], rgba[1], rgba[2]};
            if (frame.depths[index] == 0) under = bed;
            BronzeRenderSample water = bronze_render_water(under, water_height, wx, wy);
            bronze_render_pixel(pixels + index, water.r, water.g, water.b);
            frame.depths[index] = (float)depth;
        }
    }
    bronze_render_structures(&frame, land, camera, focal, horizon);
    bronze_render_trees(&frame, land, camera, focal, horizon);
    simulated_being *actor = group->select;
    if (actor) {
        double ax = APESPACE_TO_MAPSPACE(being_location_x(actor));
        double ay = APESPACE_TO_MAPSPACE(being_location_y(actor));
        double dx = remainder(ax - camera.x, MAP_DIMENSION);
        double dy = remainder(ay - camera.y, MAP_DIMENSION);
        double depth = dx * camera.fx + dy * camera.fy;
        if (depth < 2 || depth > 100) {
            free(frame.depths);
            return;
        }
        double side = -dx * camera.fy + dy * camera.fx;
        int center = (int)(width * 0.5 + side * focal / depth);
        int ground = (int)(horizon + (bronze_render_sample(land, ax, ay, 0).height - camera.eye) * focal / depth);
        int size = (int)(1.6 * focal / depth);
        if (size < 3) size = 3;
        int ring = size / 2 + 1;
        float actor_depth = (float)(depth - 0.3);
        for (int y = -ring / 2; y <= ring / 2; ++y) {
            for (int x = -ring; x <= ring; ++x) {
                int distance = x * x + 4 * y * y;
                if (distance > ring * ring || distance < (ring - 2) * (ring - 2)) continue;
                bronze_depth_pixel(&frame, center + x, ground + y, actor_depth, 250, 188, 101);
            }
        }
        /* Project the actor and ring into the same depth field as the trees. */
        for (int y = 1; y <= size; ++y) {
            int half_width = y < size / 3 ? 1 : size / 5;
            for (int x = -half_width; x <= half_width; ++x) {
                if (y < size / 3 && x == 0) continue;
                bronze_depth_pixel(&frame, center + x, ground + y, actor_depth,
                                  84 + x * 4, 62 + x * 3, 41);
            }
        }
        for (int y = size + 1; y <= size + 3; ++y) {
            for (int x = -1; x <= 1; ++x)
                bronze_depth_pixel(&frame, center + x, ground + y, actor_depth, 164, 112, 73);
        }
    }
    free(frame.depths);
}

#endif
