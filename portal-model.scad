use <../modely/esp32.scad>
use <../modely/charger-module.scad>
use <../modely/flexbatter.scad>
use <../modely/buttons.scad>
use <../modely/mfrc.scad>

//DEBUG = true;
DEBUG = false;


fatness = 1.4;
inset = .2;

shaft_inset = .4;

round_prec = 40;

// ---------------------------------------------

angle_inner = 22.95;
angle_outter = 45;

pcb_size = [60, 40, 1.6];
rtc_size = [27, 28, 9.56];
keyboard_size = [69, 76.2, 1];
keyboard_conn_size = [20.3, 19.7, 2.9];

pcf_size = [47.9, 4.25, 15.7];
pcf_header_x = 17.2;

ring_outter_dia = 67.8;
ring_inner_dia = 54.3;
ring_height = 3;

battery_length = 66;
battery_dia = 19;
batt_holder_size = [66 + 2 * 1, battery_dia + 2 * 1, battery_dia + 1];

display_board_size = [58, 1.6, 34.2];
display_size = [43.8, 2.5, display_board_size.z];
display_pos = [6.4, 0, 0];

outter_dia = 105;
inner_dia = outter_dia - 2 * fatness;
outter_height = 45;

switch_pos = [40.3, 5.9, - .01];
charger_pos = [34.85, 19.4, - .01];
charger_hole = [4.1, 10.2, 100];

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
    esp_pos = [1, 8, 11.2];
    rtc_pos = [- 23.3 + esp_pos.x, esp_pos.y + 0.2, 0];

    color("gray") cube(pcb_size);
    color("red") translate([esp_pos.x, esp_pos.y, pcb_size.z]) cube([48, 2.4, esp_pos.z]);
    color("red") translate([esp_pos.x, esp_pos.y + ESP32_size().y - 2.4, pcb_size.z]) cube([48, 2.4, esp_pos.z]);
    color("black") translate([esp_pos.x, esp_pos.y, esp_pos.z + pcb_size.z]) ESP32();
    //    color("black") translate([rtc_pos.x, rtc_pos.y, rtc_pos.z + pcb_size.z]) cube(rtc_size);
}

module PCF() {
    color("violet") cube(pcf_size);
    color("black") translate([pcf_header_x, - .2]) cube([keyboard_conn_size.x, .2, keyboard_conn_size.z]);
}

module Display() {
    union() {
        header_size = [2.5, 9, 20.2];
        color("gray") cube(display_board_size);
        color("green") translate([display_pos.x, - display_size.y, display_pos.y]) cube(display_size);
        color("gray") translate([0, - display_size.y]) rotate([90]) translate([18, 15]) text("Display", size = 5);

        color("black") translate([display_board_size.x - .7 - header_size.x, 0]) {
            translate([0, - 1, (display_board_size.z - header_size.z) / 2]) cube([header_size.x, 1, header_size.z]);
            translate([0, display_board_size.y, (display_board_size.z - header_size.z) / 2]) cube(header_size);
        }
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
        translate([1, 1, 1]) cube([66, battery_dia, 100]);

        // debug:
        // translate([-.01,-.01,-.01]) cube([1 + .02, batt_holder_size.y + .02, 100]);
    }
    if (DEBUG) translate([1, battery_dia / 2 + 1, battery_dia / 2 + 1]) color("grey") rotate([0, 90])
        cylinder(d = battery_dia, h = battery_length, $fn = 25);
}

module MainPart() {
    difference() {
        cylinder(d = outter_dia, h = outter_height, $fn = 100);
        translate([0, 0, fatness]) color("orange") cylinder(d = inner_dia, h = outter_height, $fn = 100);

        // switch hole
        translate(switch_pos) cylinder(d = switch_head_dia() + 2, h = 100, $fn = round_prec);

        // charger hole
        translate(charger_pos) cube(charger_hole);


        // DEBUG:
        // translate([0, 0, fatness]) cylinder(d = outter_dia + .01, h = outter_height + .01, $fn = 100);

        translate([0, 0, fatness + .01]) difference() {
            angle = 43;

            linear_extrude(100) sector(outter_dia / 2 + .01, [- 90 - angle, - 90 + angle], 200);
            translate([0, 0, - .01]) linear_extrude(200) sector(inner_dia / 2 - 5, [- 90 - angle - .01, - 90 + angle + .01], 200);
            translate([- 100, - .01, - .01]) cube([200, 50, 200]);
        }
    }
}

module HandlePart() {
    dia = 90;
    width = 10;

    h = fatness;

    kbed_size = [keyboard_size.x + 2 * (1 + inset), keyboard_size.y + 2 * (1 + inset), h + 2];

    union() {
        difference() {

            union() {
                difference() {
                    linear_extrude(outter_height) sector(dia, [90 - angle_outter, 90 + angle_outter], 200);
                    translate([0, 0, h - .01]) linear_extrude(100) sector(dia - width, [0, 360], 200);
                    translate([0, 0, h - .01]) linear_extrude(100) sector(dia + .01, [90 - angle_inner, 90 + angle_inner], 200);

                    translate([- keyboard_size.x / 2 - inset, 70, h + .01]) cube([keyboard_size.x + 2 * inset, 13, 100]);
                }

                translate([- kbed_size.x / 2, - 5.32, 0]) {
                    difference() {
                        cube(kbed_size);
                        translate([1, 1, fatness]) cube([keyboard_size.x + 2 * inset, keyboard_size.y + 2 * inset, 100]);

                        // connector
                        translate([(kbed_size.x - keyboard_conn_size.x) / 2 - 1, kbed_size.y - 1 - .01]) {
                            cube([keyboard_conn_size.x + 2, 1 + .02, 100]);
                        }
                    }
                }
            }

            //            translate([- kbed_size.x / 2, - 5.32, 0]) {
            //                // connector cable
            //                translate([(kbed_size.x - keyboard_conn_size.x) / 2 - 1, 1, 1]) {
            //                    cube([keyboard_conn_size.x + 2, 100, 100]);
            //                }
            //            }
        }

    }
}

module DisplayPart() {
    size = [keyboard_size.x, 6, outter_height - 1.65];
    union() {
        difference() {
            color("darkred") cube(size);

            // rail for the cover
            translate([size.x / 2, 46.5, outter_height - 4.41]) difference() {
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
                cube([keyboard_conn_size.x + 2 * inset, 100, keyboard_conn_size.z + 3 * inset]);
            }

            // rails for side-holders
            for (i = [0, 1]) {
                // this is intentionally not through - because of print
                translate([- .01 + i * (size.x - 2.5 + .02), 2, .6]) color("red") cube([2.5, 2.5, 25]);

                // debug:
                // translate([- .01 + i * (size.x - 2.5 + .02), 2, .6]) color("red") cube([2.5, 2.5, 100]);
            }

            // header space
            translate([size.x - 9, size.y - 5 + .01, 11]) cube([3, 100, 21.2]);

            // debug: front
            // translate([- .01, - .01, - .01]) cube([size.x + .02, 1.1, size.z + .02]);
        }

        if (DEBUG) translate([(size.x - display_board_size.x) / 2, display_size.y, 4.5]) Display();
    }
}

module Cover() {
    total_height = 10 + fatness;

    union() {
        difference() {
            union() {
                color("green") cylinder(d = inner_dia - inset, h = 10, $fn = 100);
                color("red") translate([0, 0, 10]) cylinder(d = outter_dia, h = fatness, $fn = 100);
            }

            // inner of cover
            color("green") translate([0, 0, - .01]) cylinder(d = inner_dia - 2, h = 7, $fn = 100);

            // space for display
            translate([- 50, - 52, - .01]) cube([100, 11.5, 7.4]);

            // debug:
            // translate([0, 0, - .01]) cylinder(d = outter_dia + .01, h = outter_height + .01, $fn = 100); // whole barrel
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

            // hole for LED ring cables
            translate([- 4.5, - 33.65, - .01]) cube([9, 4.5, 100 + fatness + .02]);
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

        if (DEBUG) translate([0, 0, total_height - ring_height + .01]) LedRing();
    }
}

module Clamp() {
    union() {
        size = [68 - 2 * inset, 4 - 2 * inset, 5.2];
        cube(size);
        h = 15;
        translate([0, 0, - h + .01]) cube([4 - 2 * inset, 4 - 2 * inset, h]);
        translate([size.x - (4 - 2 * inset), 0, - h + .01]) cube([4 - 2 * inset, 4 - 2 * inset, h]);
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

            kbed_size = [keyboard_size.x + 2 * (1 + inset), keyboard_size.y + 2 * (1 + inset), fatness + 2];
            translate([- kbed_size.x / 2, - 125.32, - .2]) {
                // connector cable
                translate([(kbed_size.x - keyboard_conn_size.x) / 2 - 1, 1, 1]) {
                    cube([keyboard_conn_size.x + 2, keyboard_size.y + 10, 100]);
                }
            }
        }

        if (DEBUG) translate([- keyboard_size.x / 2, - 124.1, 1 + inset])  Keyboard();

        translate([0, 0, fatness]) {
            // board bed
            translate([- pcb_size.x / 2 - 1 - inset, - 22.5]) {
                difference() {
                    cube([pcb_size.x + 2 + 2 * inset, pcb_size.y + 2 + 2 * inset, 10]);
                    translate([1, 1]) cube([pcb_size.x + 2 * inset, pcb_size.y + 2 * inset, 100]);

                    color("green") translate([6 + inset, - .01, - 5]) cube([pcb_size.x - 14, 1 + .02, 100]);
                }
                if (DEBUG) translate([1 + inset, 1 + inset]) Board();
            }

            // PCF holder
            translate([- 29.55, - 28.199]) union() {
                size = [pcf_size.x + 2 * (2 + inset), pcf_size.y, pcf_size.z + 1 + 4 * inset];
                difference() {
                    cube(size);
                    translate([2, - .01, - .01]) cube([pcf_size.x + 2 * inset, 100, pcf_size.z + 4 * inset]);
                }

                translate([0, pcf_size.y, - .01]) cube([5, 2.44, size.z]);
                translate([43, pcf_size.y, - .01]) cube([2, 2.44, size.z]);
                translate([50.3, pcf_size.y, - .01]) cube([2, 2.44, size.z]);

                if (DEBUG) translate([2 + inset, 0, inset]) PCF();
            }

            translate([- batt_holder_size.x / 2, 18.9, - 1]) BatteryHolder();

            // side-holders for display part
            translate([- 35.1, - 44.25]) {
                for (i = [0, 1]) translate([i * 68.05, 0]) cube([2.15, 1.8, 20]);
            }

            // switch
            translate(switch_pos) {
                height = 8.8;
                f = 2.5;

                difference() {
                    cylinder(d = switch_head_dia() + 5, h = height, $fn = round_prec);
                    translate([0, 0, - .01]) {
                        cylinder(d = switch_head_dia() + 2, h = height - f, $fn = round_prec);
                        cylinder(d = switch_hole_dia(), h = 100, $fn = round_prec);
                    }
                }

                if (DEBUG) translate([0, 0, 24]) rotate([180]) Switch(f);
            }

            // charger
            translate(charger_pos) {
                translate([- 1.8, - 5 - 2 * inset, - .01]) difference() {
                    cube([4, Charger_size().x + 2 * (1 + inset), 20]);
                    translate([1, 1 + inset]) cube([100, Charger_size().x + 2 * inset, 100]);
                }

                // right wall
                translate([4.1, - 1, - .01]) difference() {
                    cube([2.5, 12, 10]);
                    translate([- .01, 3, - 0.1]) cube([2.5 + .02, 6, 8]);
                }

                if (DEBUG) translate([Charger_size().y - .5, 13.4, Charger_size().z + .4]) rotate([180, 0, - 90]) Charger();
            }

            // clamp
            for (i = [- 1, 1]) translate([- 4 + i * 36 + - i * 4, - 40]) difference() {
                color("red") cube([8, 8, 28]);
                translate([2, 2, + .01])  color("green")cube([4 + inset, 4 + inset, 100]);
            }
        }
    }
}

CompleteMainModule();

//translate([69, 124, outter_height - 10]) // for modelling
translate([- 40, 110, 10 + fatness]) rotate([0, 180]) // for print
    Cover();

//translate([keyboard_size.x / 2, 77.4, fatness + .1]) // for modelling
//translate([0, - 15]) // for modelling, standing next
translate([- 50, 45]) rotate([90]) // for print
    DisplayPart();

//translate([34.9 + 2 * inset, 86.3, 29.7]) // for modelling
translate([- 55, 55]) rotate([90, 0, - 90]) // for print
    Clamp();
