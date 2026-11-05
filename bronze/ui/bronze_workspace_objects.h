#ifndef BRONZE_WORKSPACE_OBJECTS_H
#define BRONZE_WORKSPACE_OBJECTS_H

/* World-space structures share the terrain/tree depth buffer. Their geometry
 * never participates in ground sampling or camera ground clearance. */
typedef struct { double x, y, z; } BronzeObjectVertex;
typedef struct { double side, depth, up; } BronzeObjectView;

static double bronze_object_edge(double ax,double ay,double bx,double by,double x,double y)
{ return (x-ax)*(by-ay)-(y-ay)*(bx-ax); }

static void bronze_object_triangle_view(BronzeRenderFrame *frame, BronzeObjectView *v,
                                        double focal,double horizon,int r,int g,int b)
{
    double x[3],y[3];
    for (int i=0;i<3;++i) {
        x[i]=frame->width*0.5+v[i].side*focal/v[i].depth;
        y[i]=horizon+v[i].up*focal/v[i].depth;
    }
    double area=bronze_object_edge(x[0],y[0],x[1],y[1],x[2],y[2]);
    if (fabs(area)<1e-8) return;
    int left=(int)fmax(0,floor(fmin(x[0],fmin(x[1],x[2]))));
    int right=(int)fmin(frame->width-1,ceil(fmax(x[0],fmax(x[1],x[2]))));
    int bottom=(int)fmax(0,floor(fmin(y[0],fmin(y[1],y[2]))));
    int top=(int)fmin(frame->height-1,ceil(fmax(y[0],fmax(y[1],y[2]))));
    for (int py=bottom;py<=top;++py) for (int px=left;px<=right;++px) {
        double a=bronze_object_edge(x[1],y[1],x[2],y[2],px+0.5,py+0.5)/area;
        double c=bronze_object_edge(x[0],y[0],x[1],y[1],px+0.5,py+0.5)/area;
        double d=1-a-c;
        if (a<-1e-9 || c<-1e-9 || d<-1e-9) continue;
        double depth=1/(a/v[0].depth+d/v[1].depth+c/v[2].depth);
        double haze=fmin(0.76,depth/150);
        bronze_depth_pixel(frame,px,py,(float)depth,
            (int)(r*(1-haze)+158*haze),(int)(g*(1-haze)+172*haze),(int)(b*(1-haze)+166*haze));
    }
}

static void bronze_object_triangle(BronzeRenderFrame *frame,BronzeRenderCamera camera,
                                  double focal,double horizon,BronzeObjectVertex a,
                                  BronzeObjectVertex b,BronzeObjectVertex c,int r,int g,int blue)
{
    BronzeObjectVertex world[3]={a,b,c}; BronzeObjectView input[3],clipped[4];
    for (int i=0;i<3;++i) {
        /* Objects have already been unwrapped together around their anchor. */
        double dx=world[i].x-camera.x,dy=world[i].y-camera.y;
        input[i]=(BronzeObjectView){-dx*camera.fy+dy*camera.fx,
                                    dx*camera.fx+dy*camera.fy,world[i].z-camera.eye};
    }
    int count=0;
    for (int i=0;i<3;++i) {
        BronzeObjectView p=input[i],q=input[(i+1)%3];
        if (p.depth>=1.1) clipped[count++]=p;
        if ((p.depth>=1.1)!=(q.depth>=1.1)) {
            double t=(1.1-p.depth)/(q.depth-p.depth);
            clipped[count++]=(BronzeObjectView){p.side+t*(q.side-p.side),1.1,p.up+t*(q.up-p.up)};
        }
    }
    for (int i=1;i+1<count;++i) {
        BronzeObjectView triangle[3]={clipped[0],clipped[i],clipped[i+1]};
        bronze_object_triangle_view(frame,triangle,focal,horizon,r,g,blue);
    }
}

static void bronze_object_quad(BronzeRenderFrame *frame,BronzeRenderCamera camera,
                               double focal,double horizon,BronzeObjectVertex a,
                               BronzeObjectVertex b,BronzeObjectVertex c,BronzeObjectVertex d,
                               int r,int g,int blue)
{
    bronze_object_triangle(frame,camera,focal,horizon,a,b,c,r,g,blue);
    bronze_object_triangle(frame,camera,focal,horizon,a,c,d,r,g,blue);
}

static void bronze_object_box(BronzeRenderFrame *frame,BronzeRenderCamera camera,
                              double focal,double horizon,double x,double y,double z,
                              double w,double length,double height,int r,int g,int blue)
{
    BronzeObjectVertex base[4]={{x,y,z},{x+w,y,z},{x+w,y+length,z},{x,y+length,z}},top[4];
    for (int i=0;i<4;++i) { top[i]=base[i]; top[i].z+=height; }
    for (int i=0;i<4;++i) {
        int shade=i*7;
        bronze_object_quad(frame,camera,focal,horizon,base[i],base[(i+1)%4],top[(i+1)%4],top[i],r-shade,g-shade,blue-shade);
    }
    bronze_object_quad(frame,camera,focal,horizon,top[0],top[1],top[2],top[3],r+8,g+8,blue+8);
}

static void bronze_render_structures(BronzeRenderFrame *frame,const BronzeRenderLand *land,
                                     BronzeRenderCamera camera,double focal,double horizon)
{
    if (land->settlement) for (unsigned i=0;i<land->settlement->building_count;++i) {
        const BronzeBuilding *building=&land->settlement->buildings[i];
        double x=camera.x+remainder(building->cells[0].x-camera.x,MAP_DIMENSION);
        double y=camera.y+remainder(building->cells[0].y-camera.y,MAP_DIMENSION);
        if (hypot(x-camera.x,y-camera.y)>120) continue;
        double ground=bronze_render_sample(land,x+1,y+1,0).height;
        /* A level floor rests on the highest footprint corner; foundations
         * extend down through the slope instead of warping the roof. */
        double floor=ground;
        for (int corner=0;corner<4;++corner)
            floor=fmax(floor,bronze_render_sample(land,x+(corner&1)*2,y+(corner>>1)*2,0).height);
        if (building->kind==BRONZE_BUILDING_HOME) {
            enum { sides=16 };
            BronzeObjectVertex peak={x+1,y+1,floor+2.35};
            for (int side=0;side<sides;++side) {
                double a=side*6.283185307179586/sides,b=(side+1)*6.283185307179586/sides;
                BronzeObjectVertex p={x+1+0.87*cos(a),y+1+0.87*sin(a),ground-0.8};
                BronzeObjectVertex q={x+1+0.87*cos(b),y+1+0.87*sin(b),ground-0.8};
                BronzeObjectVertex u={p.x,p.y,floor+1},v={q.x,q.y,floor+1};
                int shade=(int)(14*cos(a)+9*sin(a));
                bronze_object_quad(frame,camera,focal,horizon,p,q,v,u,143+shade,114+shade,76+shade);
                u.x=x+1+cos(a);u.y=y+1+sin(a);
                v.x=x+1+cos(b);v.y=y+1+sin(b);
                bronze_object_triangle(frame,camera,focal,horizon,u,v,peak,173+shade,136+shade,73+shade);
            }
            double dx=bronze_render_delta(building->entrance.x,building->cells[0].x)+0.5-1;
            double dy=bronze_render_delta(building->entrance.y,building->cells[0].y)+0.5-1;
            double length=hypot(dx,dy); if (length<0.01) length=1;
            dx/=length; dy/=length;
            double cx=x+1+dx*0.88,cy=y+1+dy*0.88;
            BronzeObjectVertex a={cx-dy*0.23,cy+dx*0.23,floor};
            BronzeObjectVertex b={cx+dy*0.23,cy-dx*0.23,floor};
            BronzeObjectVertex c=b,d=a; c.z+=0.82;d.z+=0.82;
            bronze_object_quad(frame,camera,focal,horizon,a,b,c,d,49,36,24);
        } else if (building->kind==BRONZE_BUILDING_STORE) {
            bronze_object_box(frame,camera,focal,horizon,x+0.12,y+0.12,ground-0.8,1.76,1.76,floor-ground+1.9,145,112,71);
            BronzeObjectVertex a={x,y,floor+1.1},b={x+2,y,floor+1.1},c={x+2,y+2,floor+1.1},d={x,y+2,floor+1.1};
            BronzeObjectVertex e={x+1,y,floor+2.1},f={x+1,y+2,floor+2.1};
            bronze_object_quad(frame,camera,focal,horizon,a,e,f,d,170,131,70);
            bronze_object_quad(frame,camera,focal,horizon,e,b,c,f,145,106,57);
            bronze_object_triangle(frame,camera,focal,horizon,a,b,e,154,120,77);
            bronze_object_triangle(frame,camera,focal,horizon,d,c,f,140,107,68);
        } else if (building->kind==BRONZE_BUILDING_WORK_YARD) {
            int metal=building->profession==BRONZE_PREMISE_METAL;
            int pottery=building->profession==BRONZE_PREMISE_POTTERY;
            bronze_object_box(frame,camera,focal,horizon,x+0.6,y+0.6,ground,0.8,0.8,
                              metal?0.65:0.45,metal?69:155,metal?61:91,metal?48:50);
            if (pottery) bronze_object_box(frame,camera,focal,horizon,x+0.2,y+1.3,ground,0.35,0.35,0.5,170,94,54);
        }
    }
    for (unsigned i=0;i<land->count;++i) {
        const BronzeRenderSite *site=&land->sites[i];
        if (site->kind!=BRONZE_COPPER_MINE && site->kind!=BRONZE_TIN_MINE) continue;
        double x=camera.x+remainder(site->x-camera.x,MAP_DIMENSION);
        double y=camera.y+remainder(site->y-camera.y,MAP_DIMENSION);
        if (hypot(x-camera.x,y-camera.y)>120) continue;
        double ground=bronze_render_sample(land,x,y,0).height;
        /* Surface mine shelter: dark entry, timber posts and a lintel. */
        bronze_object_box(frame,camera,focal,horizon,x-0.8,y-0.15,ground,1.6,0.5,1.25,45,39,31);
        for (int post=0;post<2;++post)
            bronze_object_box(frame,camera,focal,horizon,x-1+post*1.8,y-0.6,ground,0.18,1.2,1.4,102,73,43);
        bronze_object_box(frame,camera,focal,horizon,x-1,y-0.6,ground+1.3,2,1.2,0.2,119,87,49);
        int copper=site->kind==BRONZE_COPPER_MINE;
        for (int rock=0;rock<5;++rock)
            bronze_object_box(frame,camera,focal,horizon,x+1.15+(rock%2)*0.35,y-0.6+rock*0.3,ground,
                              0.4,0.4,0.2+(rock%3)*0.12,copper?131:115,copper?88:119,copper?57:117);
    }
}
#endif
