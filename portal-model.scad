use <../modely/esp32.scad>
use <../modely/charger-module.scad>
use <../modely/flexbatter.scad>
use <../modely/buttons.scad>
use <../modely/mfrc.scad>

DEBUG = true;
//DEBUG = false;


fatness = 1.4;
inset = .2;

shaft_inset = .4;

round_prec = 40;

// ---------------------------------------------

pcb_size = [60, 80, 1.6];
rtc_size = [27, 28, 9.56];
keyboard_size = [69, 76, 1];
keyboard_conn_size = [20.3, 14.9, 2.9];

ring_outter_dia = 67.8;
ring_inner_dia = 54.3;
ring_height = 3;

battery_length = 66;
battery_dia = 19;
batt_holder_size = [66 + 2 * 1.5, battery_dia + 2 * 1.5, battery_dia + 1.5];

display_board_size = [58, 1.6, 34.2];
display_size = [43.8, 2.5, display_board_size.z];
display_pos = [6.4, 0, 0];

outter_dia = 105;
inner_dia = outter_dia - 2 * fatness;
outter_height = 45;

switch_pos = [41, 5, - .01];

// ---------------------------------------------

module sector(radius, angles, fn = 24) {
    r = radius / cos(180 / fn);
    step = - 360 / fn;

    points = concat([[0, 0]],
    [for (a = [angles[0] : step : angles[1] - 360])
        [r * cos(a), r * sin(a)]
    ],
        [[r * cos(angles[1]), r * sin(angles[1])]]
    );

    difference() {
        circle(radius, $fn = fn);
        polygon(points);
    }
}

module arc(radius, angles, width = 1, fn = 24) {
    difference() {
        sector(radius + width, angles, fn);
        sector(radius, angles, fn);
    }
}

module Board() {
    esp_pos = [17, 8, 11.2]; // TODO
    rtc_pos = [- 23.3 + esp_pos.x, esp_pos.y + 0.2, 0];

    color("gray") cube(pcb_size);
    color("red") translate([esp_pos.x + 3.8, esp_pos.y, pcb_size.z]) cube([38.5, 2.4, esp_pos.z]);
    color("red") translate([esp_pos.x + 3.8, esp_pos.y + ESP32_size().y - 2.4, pcb_size.z]) cube([38.5, 2.4, esp_pos.z]);
    color("black") translate([esp_pos.x, esp_pos.y, esp_pos.z + pcb_size.z]) ESP32();
    color("black") translate([rtc_pos.x, rtc_pos.y, rtc_pos.z + pcb_size.z]) cube(rtc_size);
}

module Display() {
    union() {
        header_size = [2.5, 9, 20.2];
        color("gray") cube(display_board_size);
        color("green") translate([display_pos.x, - display_size.y, display_pos.y]) cube(display_size);
        color("black") translate([.7, display_board_size.y, (display_board_size.z - header_size.z) / 2]) cube(header_size);
        color("gray") translate([0, - display_size.y]) rotate([90]) translate([18, 15]) text("Display", size = 5);
    }
}

module LedRing() {
    difference() {
        color("blue") cylinder(d = ring_outter_dia, h = ring_height, $fn = 50);
        translate([0, 0, - .01]) cylinder(d = ring_inner_dia, h = 100, $fn = 50);
    }
}

module Keyboard() {
    color("black") cube(keyboard_size);
    color("grey") translate([(keyboard_size.x - keyboard_conn_size.x) / 2, keyboard_size.y]) cube(keyboard_conn_size);
}

module BatteryHolder() {
    difference() {
        cube(batt_holder_size);
        translate([1.5, 1.5]) cube([66, battery_dia, 100]);
    }
    if (DEBUG) translate([1.5, battery_dia / 2 + 1.5, battery_dia / 2 + 1.5]) color("grey") rotate([0, 90])
        cylinder(d = battery_dia, h = battery_length, $fn = 25);
}

module MainPart() {
    union() {
        difference() {
            cylinder(d = outter_dia, h = outter_height, $fn = 100);
            translate([0, 0, fatness]) cylinder(d = inner_dia, h = outter_height, $fn = 100);

            // switch hole
            translate(switch_pos) {
                cylinder(d = switch_head_dia() + 2, h = 100, $fn = round_prec);
            }

            // DEBUG:
            // translate([0, 0, fatness]) cylinder(d = outter_dia + .01, h = outter_height + .01, $fn = 100);
        }
    }
}

module HandlePart() {
    dia = 90;
    width = 10;

    angle = 50;

    difference() {
        linear_extrude(outter_height) sector(dia, [90 - angle, 90 + angle], 200);
        translate([0, 0, fatness]) linear_extrude(100) sector(dia - width, [0, 360], 200);
    }
}

module DisplayPart() {
    size = [69.7, 6, outter_height - 1.6];
    union() {
        difference() {
            color("darkred") cube(size);

            // rail for the cover
            translate([34.9, 48.1, outter_height - 4.41]) difference() {
                cylinder(d = inner_dia, h = 50, $fn = 100);
            }

            // display hole
            translate([(size.x - display_board_size.x) / 2 + display_pos.x - inset, - .01, 4.3]) {
                cube([display_size.x + 2 * inset, display_size.y + 2 * inset, display_size.z + 2 * inset]);
                translate([- display_pos.x, display_size.y - inset])
                    cube([display_board_size.x + 2 * inset, 100, display_board_size.z + 2 * inset]);
            }

            // keyboard connector hole
            translate([(size.x - keyboard_conn_size.x) / 2 - inset, - .01, - .01]) {
                cube([keyboard_conn_size.x + 2 * inset, 100, keyboard_conn_size.z + inset]);
            }

            // rails for side-holders
            for (i = [0, 1]) {
                translate([- .01 + i * (size.x - 2 + .02), 2.2, - 5]) color("red") cube([2, 2, 25]);
            }
        }

        if (DEBUG) translate([(size.x - display_board_size.x) / 2, display_size.y, 4.5]) Display();
    }
}

module Cover() {
    total_height = 10 + fatness;

    union() {
        difference() {
            union() {
                color("green") cylinder(d = inner_dia - 2 * inset, h = 10, $fn = 100);
                color("red") translate([0, 0, 10]) cylinder(d = outter_dia, h = fatness, $fn = 100);
            }

            // inner of cover
            color("green") translate([0, 0, - .01]) cylinder(d = inner_dia - 2, h = 7, $fn = 100);

            // space for display
            translate([- 50, - 52, - .01]) cube([100, 10, 7.4]);

            // debug:
            translate([0, 0, - .01]) cylinder(d = outter_dia + .01, h = outter_height + .01, $fn = 100); // whole barrel
            // translate([0, 0, 6.98]) cylinder(d = outter_dia + .01, h = outter_height + .01, $fn = 100); // just top to NFC
            // translate([0, 0, 5]) color("green") cylinder(d = outter_dia + .01, h = outter_height + .01, $fn = 100); // just top
            // translate([-100,-60 - .01]) cube([200, 30, 100]);  // front wall

            // rail for LED ring
            translate([0, 0, total_height - ring_height]) {
                difference() {
                    cylinder(d = ring_outter_dia + .8, h = 100, $fn = 50);
                    cylinder(d = ring_inner_dia - .8, h = 100, $fn = 50);
                }

                translate([0, 0, 2]) cylinder(d = ring_inner_dia - 2.5, h = 100, $fn = 50);
            }
        }

        // NFC holder
        translate([- MFRC_board_size().x / 2 - 10, - MFRC_board_size().y / 2, 2.01]) {
            outter = [MFRC_board_size().x + 2 * (1 + inset), MFRC_board_size().y + 2 * (1 + inset), 5];

            difference() {
                cube(outter);
                translate([1, 1, - .01]) cube([MFRC_board_size().x + 2 * inset, MFRC_board_size().y + 2 * inset, 100]);
            }

            if (DEBUG) translate([1 + inset, 1 + inset + MFRC_board_size().y, 5]) rotate([180]) MFRC_board();
        }

        //        if (DEBUG) translate([0, 0, total_height - ring_height + .01]) LedRing();
    }
}

// ---------------------------------------------

module CompleteMainModule() {
    translate([69, 124]) union() {
        difference() {
            union() {
                MainPart();
                translate([0, - 120]) HandlePart();
            }


            translate([- 35, - 55, fatness]) cube([70, 30, 100]);
        }

        translate([0, 0, fatness]) {
            // board bed
            translate([- pcb_size.x / 2 - 1 - inset, - 40.2]) {
                difference() {
                    cube([pcb_size.x + 2 + 2 * inset, pcb_size.y + 2 + 2 * inset, 5]);
                    translate([1, 1]) cube([pcb_size.x + 2 * inset, pcb_size.y + 2 * inset, 100]);
                }
                if (DEBUG) translate([1 + inset, 1 + inset]) Board();
            }

            // battery holder holder :-)
            translate([- (batt_holder_size.x + 2 * (1 + inset)) / 2, 14]) difference() {
                cube([batt_holder_size.x + 2 * (1 + inset), batt_holder_size.y + 2 * (1 + inset), 15]);
                translate([1, 1]) cube([batt_holder_size.x + 2 * (inset), batt_holder_size.y + 2 * (inset), 100]);
                translate([(batt_holder_size.x - pcb_size.x) / 2, - .01]) {
                    cube([pcb_size.x + 2 * (1 + inset), 100, 100]);
                }
            }

            // side-holders for display part
            translate([- 35.1, - 45.79]) {
                for (i = [0, 1]) translate([i * 68.05, 0]) cube([2.15, 1.8, 20]);
            }

            // switch
            translate(switch_pos) {
                height = 7;

                difference() {
                    cylinder(d = switch_head_dia() + 5, h = height, $fn = round_prec);
                    translate([0, 0, - .01]) {
                        cylinder(d = switch_head_dia() + 2, h = height - fatness, $fn = round_prec);
                        cylinder(d = switch_hole_dia(), h = 100, $fn = round_prec);
                    }
                }

                if (DEBUG) translate([0, 0, height + 16.35]) rotate([180]) Switch(fatness);
            }


            if (DEBUG) translate([- keyboard_size.x / 2, - 124]) Keyboard();
        }
    }
}

CompleteMainModule();

translate([69, 124]) translate([- batt_holder_size.x / 2, 15.2, 7]) // for modelling
    //translate([150, 80]) rotate([0, 0, 90]) // for print
    BatteryHolder();

translate([69, 124, outter_height - 10]) // for modelling
    //    translate([outter_dia / 2, - 80]) rotate([0, 180]) // for print
    Cover();

translate([34.15, 75.9, fatness + .01]) // for modelling
    //    translate([0, - 15]) // for modelling, standing next
    //translate([0, -5]) rotate([90]) // for print
    DisplayPart();
