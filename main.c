#include <dos.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/nearptr.h>
#include <string.h>
#include <math.h>
#include <time.h>

typedef unsigned char byte;

#define sgn(x) ((x<0)?-1:((x>0)?1:0))

#define FIXED_POINT_LOCATION 1000
#define PIXEL_SCALE 70

//#define SCREEN_WIDTH 320

byte *VGA = (byte *) 0xA0000;
volatile short *hw_clock = (short *)0x046C;

typedef int Vec2[2];

typedef Vec2 Pair;

typedef int Vec3[3];

typedef int Transform4[4][4];

typedef int Transform4to3[3][4];

typedef int Vec4[4];

const Transform4 IDENTITY4 = {
    {FIXED_POINT_LOCATION, 0, 0, 0},
    {0, FIXED_POINT_LOCATION, 0, 0},
    {0, 0, FIXED_POINT_LOCATION, 0},
    {0, 0, 0, FIXED_POINT_LOCATION}
};


void set_mode(byte mode) {
    union REGS regs;
    regs.h.ah = 0x00;
    regs.h.al = mode;

    int86(0x10, &regs, &regs);
}

void init_mouse() {
    union REGS regs;

    regs.x.ax = 0;
    int86(0x33, &regs, &regs);
    //mouse_on = regs.x.ax;
    //num_buttons = regs.x.bx;
}

void get_mouse_pos(Vec2 out) {
    union REGS regs;
    
    regs.x.ax= 0x03;
    int86(0x33, &regs, &regs);

    out[0] = regs.x.cx >> 1;
    out[1] = regs.x.dx;
}

byte get_key() {
    union REGS regs;
    
    regs.h.ah = 0x00;

    int86(0x16, &regs, &regs);

    return regs.h.ah;

}


void set_pixel(int x, int y, byte color) {
    
    //if (x < 0 || x >= 320 || y < 0 || y >= 200) {
    //    return;
    //}

    VGA[(y<<8)+(y<<6)+x] = color;
}

void draw_line(int x1, int y1, int x2, int y2, byte color) {
    int dx = x2 - x1;
    int dy = y2 - y1;

    int dxabs = abs(dx);
    int dyabs = abs(dy);

    int sdx = sgn(dx);
    int sdy = sgn(dy);

    set_pixel(x1, y1, color);

    int x = dxabs >> 1;
    int y = dyabs >> 1;

    int px = x1;
    int py = y1;

    if (dxabs > dyabs) {
        for (int i = 0; i < dxabs; i++) {
            y += dyabs;
            if (y >= dxabs) {
                y -= dxabs;
                py += sdy;
            }

            px += sdx;
            set_pixel(px, py, color);
        }
    } else {
        for (int i = 0; i < dyabs; i++) {
            x += dxabs;
            if (x >= dyabs) {
                x -= dyabs;
                px += sdx;
            }

            py += sdy;
            set_pixel(px, py, color);
        }
    }
}

void draw_line_point(Vec3 p0, Vec3 p1, byte color) {

    //if (p0[2] == 0) { p0[2] = 1; }
    //if (p1[2] == 0) { p1[2] = 1; }

    if ((p0[2] <= 0) || (p1[2] <= 0)) { return; }

    int x0, y0, x1, y1;
    x0 = PIXEL_SCALE * p0[0] / p0[2] + 160;
    y0 = PIXEL_SCALE * p0[1] / p0[2] + 100;
    x1 = PIXEL_SCALE * p1[0] / p1[2] + 160;
    y1 = PIXEL_SCALE * p1[1] / p1[2] + 100;

    if (x0 < 0 || x0 >= 320 || x1 < 0 || x1 >= 320 || y0 < 0 || y0 >= 200 || y1 < 0 || y1 >= 200) {
        return;
    } 

    draw_line(x0, y0, x1, y1, color);
}

void draw_rectangle(int x0, int y0, int x1, int y1, byte color) {
    draw_line(x0, y0, x1, y0, color);
    draw_line(x0, y0, x0, y1, color);
    draw_line(x1, y1, x1, y0, color);
    draw_line(x1, y1, x0, y1, color);
}

/*
void draw_lines(Vec3 points[], Pair lines[], byte colors[], int line_count, int num_points) {
    int ip0, ip1;
    for (int i = 0; i < line_count; i++){

        ip0 = lines[i][0];
        ip1 = lines[i][1];

        if (ip0 < 0 || ip0 > num_points) {
            printf("Bruh. %d", ip0);
            exit(0);
        } else if (ip1 < 0 || ip1 > num_points) {
            printf("Bruh? %d", ip1);
            exit(0);
        } else {
            draw_line_point(points[lines[i][0]], points[lines[i][1]], colors[i]);
        }
    }
}*/

void draw_lines_one_color(Vec3 points[], Pair lines[], byte color, int line_count, int num_points) {
    int ip0, ip1;
    for (int i = 0; i < line_count; i++){

        ip0 = lines[i][0];
        ip1 = lines[i][1];

        if (ip0 < 0 || ip0 >= num_points) {
            printf("Bruh. %d", ip0);
            exit(0);
        } else if (ip1 < 0 || ip1 >= num_points) {
            printf("Bruh? %d", ip1);
            exit(0);
        } else {
            draw_line_point(points[lines[i][0]], points[lines[i][1]], color);
        }
    }
}


void project_points(Vec4 points[], Vec2 out_points[], int num_points, int focal_length) {

    for (int i = 0; i < num_points; i++) {
        if (points[i][2] == 0) { points[i][2] = 1; }
        out_points[i][0] = focal_length * points[i][0] / points[i][2];
        out_points[i][1] = focal_length * points[i][1] / points[i][2];
    }
}

void scale_point(Vec4 point, int scale) {
    point[0] *= scale;
    point[1] *= scale;
    point[2] *= scale;
    point[3] *= scale;
}

void add_point(Vec4 point, int scalar) {
    point[0] += scalar;
    point[1] += scalar;
    point[2] += scalar;
}

void clear_screen() {
    memset(VGA, 0, 320 * 200);
}

void shift_points(Vec4 points[], Vec4 shift, int num_points, Vec4 out[]){
    for(int i = 0; i < num_points; i++) {
        out[i][0] = points[i][0] + shift[0];
        out[i][1] = points[i][1] + shift[1];
        out[i][2] = points[i][2] + shift[2];
    }
}

/*
 * Does a matrix mul on a (lxm) and b (mxn), and stores the result in out
 */
void mat_mul(int l, int m, int n, int a[l][m], int b[m][n], int out[l][n]) {
    for (int i = 0; i < l; i++) {
        for (int j = 0; j < n; j++) {
            out[i][j] = 0;
            for (int k = 0; k < m; k++) {
               out[i][j] += (a[i][k] * b[k][j])/FIXED_POINT_LOCATION; 
            }
        }
    }
}

// does a mat mul with the second operand transposez and the output transpose
void mat_mul_t(int l, int m, int n, int a[l][m], int b[n][m], int out[n][l]) {
    for (int i = 0; i < l; i++) {
        for (int j = 0; j < n; j++) {
            out[j][i] = 0;
            for (int k = 0; k < m; k++) {
               out[j][i] += (a[i][k] * b[j][k])/FIXED_POINT_LOCATION; 
            }
        }
    }
}

//TODO: inline this
void apply_4x4_transformation(Transform4 transform, Vec4 points[], int num_points, Vec4 out[]) {
    mat_mul_t(4, 4, num_points, transform, points, out);
}

void apply_4x3_transformation(Transform4to3 transform, Vec4 points[], int num_points, Vec3 out[]) {
    mat_mul_t(3, 4, num_points, transform, points, out);
}

void displacement_matrix(Vec3 displacement, Transform4 out) { 
    memcpy(out, IDENTITY4, sizeof(Transform4));
    out[0][3] = displacement[0];
    out[1][3] = displacement[1];
    out[2][3] = displacement[2];
}

void rotation_matrix(Vec3 angles, Transform4 out) {
    
    // TODO: do this with integer arithmetic and not as ugly
    int sina = (int) (sin(((double) angles[0])/FIXED_POINT_LOCATION) * FIXED_POINT_LOCATION);
    int cosa = (int) (cos(((double) angles[0])/FIXED_POINT_LOCATION) * FIXED_POINT_LOCATION);
    
    int sinb = (int) (sin(((double) angles[1])/FIXED_POINT_LOCATION) * FIXED_POINT_LOCATION);
    int cosb = (int) (cos(((double) angles[1])/FIXED_POINT_LOCATION) * FIXED_POINT_LOCATION);

    int siny = (int) (sin(((double) angles[2])/FIXED_POINT_LOCATION) * FIXED_POINT_LOCATION);
    int cosy = (int) (cos(((double) angles[2])/FIXED_POINT_LOCATION) * FIXED_POINT_LOCATION);
    
    memcpy(out, IDENTITY4, sizeof(Transform4));

    out[0][0] = (cosa * cosb) / FIXED_POINT_LOCATION;
    out[0][1] = ((cosa * sinb * siny) / FIXED_POINT_LOCATION - (sina * cosy)) / FIXED_POINT_LOCATION;
    out[0][2] = ((cosa * sinb * cosy) / FIXED_POINT_LOCATION + (sina * siny)) / FIXED_POINT_LOCATION;
 
    out[1][0] = (sina * cosb) / FIXED_POINT_LOCATION;
    out[1][1] = ((sina * sinb * siny) / FIXED_POINT_LOCATION + (cosa * cosy)) / FIXED_POINT_LOCATION;
    out[1][2] = ((sina * sinb * cosy) / FIXED_POINT_LOCATION - (cosa * siny)) / FIXED_POINT_LOCATION;
    
    out[2][0] = -sinb;
    out[2][1] = (sina * cosb) / FIXED_POINT_LOCATION;
    out[2][2] = (cosa * cosb) / FIXED_POINT_LOCATION;
    
}

void camera_matrix(Vec3 displacement, Vec3 angles, Transform4to3 out) {

    // TODO: do this with integer arithmetic and not as ugly
    int sina = (int) (sin(((double) angles[0])/FIXED_POINT_LOCATION) * FIXED_POINT_LOCATION);
    int cosa = (int) (cos(((double) angles[0])/FIXED_POINT_LOCATION) * FIXED_POINT_LOCATION);
    
    int sinb = (int) (sin(((double) angles[1])/FIXED_POINT_LOCATION) * FIXED_POINT_LOCATION);
    int cosb = (int) (cos(((double) angles[1])/FIXED_POINT_LOCATION) * FIXED_POINT_LOCATION);

    int siny = (int) (sin(((double) angles[2])/FIXED_POINT_LOCATION) * FIXED_POINT_LOCATION);
    int cosy = (int) (cos(((double) angles[2])/FIXED_POINT_LOCATION) * FIXED_POINT_LOCATION);

    // rotation matrix
    out[0][0] = (cosa * cosb) / FIXED_POINT_LOCATION;
    out[0][1] = ((cosa * sinb * siny) / FIXED_POINT_LOCATION - (sina * cosy)) / FIXED_POINT_LOCATION;
    out[0][2] = ((cosa * sinb * cosy) / FIXED_POINT_LOCATION+ (sina * siny)) / FIXED_POINT_LOCATION;
    
    out[1][0] = (sina * cosb) / FIXED_POINT_LOCATION;
    out[1][1] = ((sina * sinb * siny) / FIXED_POINT_LOCATION + (cosa * cosy)) / FIXED_POINT_LOCATION;
    out[1][2] = ((sina * sinb * cosy) / FIXED_POINT_LOCATION - (cosa * siny)) / FIXED_POINT_LOCATION;
    
    out[2][0] = -sinb;
    out[2][1] = (cosb * siny) / FIXED_POINT_LOCATION;
    out[2][2] = (cosb * cosy) / FIXED_POINT_LOCATION;

    // displacement vector
    out[0][3] = displacement[0];
    out[1][3] = displacement[1];
    out[2][3] = displacement[2];
}

//TODO: inline this
void compose4(Transform4 a, Transform4 b, Transform4 out) {
    mat_mul(4, 4, 4, a, b, out);
}

void printTransform4(Transform4 a) {
    for(int i = 0; i < 4; i++) {
        printf("%d %d %d %d\n", a[i][0], a[i][1], a[i][2], a[i][3]);
    }
}

void printTransform4to3(Transform4to3 a) {
    for(int i = 0; i < 3; i++) {
        printf("%d %d %d %d\n", a[i][0], a[i][1], a[i][2], a[i][3]);
    }
}

void printVec4(Vec4 a){
    printf("%d, %d, %d, %d\n", a[0], a[1], a[2], a[3]);
}
void printVec3(Vec3 a){
    printf("%d, %d, %d\n", a[0], a[1], a[2]);
}

typedef struct {
    int num_points;
    int num_lines;
    Vec4* points;
    Pair* lines;
} Obj3d;

/* error codes:
 * -1: file could not be opened
 * -2: could not store file
*/
int ReadPmfFile(char* filename, Obj3d *out) {

    FILE* f;
    int size;

    f = fopen(filename, "rb");
    
    if (f == NULL) {
        return -1;
    }
    
    fseek(f, 0, SEEK_END); // seek to end of file
    size = ftell(f); // get current file pointer
    fseek(f, 0, SEEK_SET); // seek back to beginning of file
    
    void* buffer = malloc(size - sizeof(int) * 2);
    if (buffer == NULL) { 
        return -2;
    }

    //printf("made buffer at %p\n", buffer);
    

    fread(out, sizeof(int), 2, f);
    fread(buffer, 1, size - sizeof(int) * 2, f);

    //printf("hi buffer\n");

    out->points = buffer;
    out->lines = buffer + sizeof(Vec4) * out->num_points;

    return 0;
}

void FreePmfFile(Obj3d obj){
    free(obj.points);
}

int main(int argc, char** argv) {

    if (argc < 2) {
        printf("Need a file to show\n");
        exit(-1);
    }

    Obj3d object;
    int status = ReadPmfFile(argv[1], &object);

    if (status == -1) {
        printf("Could not find the file \"%s\"\n", argv[1]);
        exit(-1);
    } else if (status == -2) {
        printf("Could not allocate the memory needed to store the file\n");
        exit(-1);
    } else if (status) {
        printf("Smth fucked.\n");
        exit(-1);
    }

    if (__djgpp_nearptr_enable() == 0) {
        printf("Could get access to first 640K of memory.\n");
        exit(-1);
    }

    VGA += __djgpp_conventional_base;
    hw_clock = (void *)hw_clock + __djgpp_conventional_base;
    
    Vec3 camera_position = {0, 0, 1000};
    Vec3 camera_rotation = {0, 0, 0}; //{1000, 1000, 1000};
    Transform4to3 camera_transform;

    Vec3* proj_points = malloc((object.num_points) * sizeof(Vec4));
    if (proj_points == NULL) {
        printf("Could not allocate the memory needed to store the projected points\n");
    }

    byte color = 43;

    set_mode(0x13);

    //int start, calc_time, render_time;
    
    while (1) { 
        

        //start = *hw_clock;
        
        camera_matrix(camera_position, camera_rotation, camera_transform); 

        apply_4x3_transformation(camera_transform, object.points, object.num_points, proj_points);

        //calc_time = *hw_clock - start;

        clear_screen();

        //printTransform4to3(camera_transform);
        
        draw_lines_one_color(proj_points, object.lines, color, object.num_lines, object.num_points);

        //render_time = *hw_clock - calc_time - start;

        //printf("%d ticks\n%d ticks\n", calc_time, render_time);

        byte key = get_key();

        if (key == 0x01) { // esc
            break;
        } else if (key == 0x1E) { // A
            camera_position[0] += 100;
        } else if (key == 0x20) { // D
            camera_position[0] += -100;
        } else if (key == 0x10) { // Q
            camera_position[1] += 100;
        } else if (key == 0x12) { // E
            camera_position[1] += -100;
        } else if (key == 0x11) { // W
            camera_position[2] += -100;
        } else if (key == 0x1F) { // S
            camera_position[2] += 100;
        } else if (key == 0x24) { // j
            camera_rotation[0] += 100;
        } else if (key == 0x26) { // l
            camera_rotation[0] += -100;
        } else if (key == 0x16) { // u
            camera_rotation[1] += 100;
        } else if (key == 0x18) { // o
            camera_rotation[1] += -100;
        } else if (key == 0x17) { // i
            camera_rotation[2] += -100;
        } else if (key == 0x25) { // k
            camera_rotation[2] += 100;
        }

     }

    set_mode(0x3);

}


