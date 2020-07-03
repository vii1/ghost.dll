program ghost_demo;
import "ghost.dll";
global
    fpg;

private
    a,b,i,j;
    wri_status;
    blend = 1;

begin
    set_mode(m640x480);
    ghost_select(1);
    fpg = load_fpg("ghost.fpg");
    a = new_map(256,256,0,0,0);
    b = new_map(256,256,0,0,0);
    from i=0 to 255;
        from j=0 to 255;
            map_put_pixel(0, a, j, i, i);
            map_put_pixel(0, b, j, i, j);
        end
    end
    put(0,a,0,0);
    xput(0,b,0,0,0,100,4,0);

    from i=0 to 31;
        particula(i);
    end
    sprite_rgb(0);
    sprite_rgb(1);
    sprite_rgb(2);

    wri_status = write(0, 1, 450, 6, "Additive blending is: ENABLED");
    write(0, 1, 460, 6, "Press SPACE to toggle");
    write(0, 1, 478, 6, "Press ALT+X to exit");

    loop
        explosion(rand(100,150), rand(320,370));
        explosion(rand(100,150), rand(320,370));

        if(key(_space))
            delete_text(wri_status);
            if(!blend)
                blend = 1;
                wri_status = write(0, 1, 450, 6, "Additive blending is: ENABLED");
            else
                blend = 0;
                wri_status = write(0, 1, 450, 6, "Additive blending is: DISABLED");
            end
            ghost_select(blend);
            put(0,a,0,0);
            xput(0,b,0,0,0,100,4,0);
        end
        frame;
    end
end

process particula(n)
private
    a;
begin
    file = fpg;
    graph=1;
    angle=rand(0,2*PI);
    a = rand(0,2*pi);
    flags = 4;
    resolution = 100;
    loop
        x = 50000 + get_distx(11250*n, sin(a) * 5);
        y = 12000 + get_disty(11250*n, sin(a) * 5);
        a += 2000;
        frame;
    end
end

process sprite_rgb(n)
private
    a;
begin
    file = fpg;
    graph = n+2;
    resolution = 100;
    flags = 4;
    loop
        x = 50000 + get_distx(120000*n+a, sin(a) * 3);
        y = 32000 + get_disty(120000*n+a, sin(a) * 3);
        a += 2000;
        frame;
    end
end

process explosion(x, y)
begin
    file = fpg;
    angle = rand(0, 2*PI);
    flags = 4;
    from graph=5 to 14;
        frame;
    end
end