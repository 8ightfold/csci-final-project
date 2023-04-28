#include <api/console.hpp>
#include <api/framebuffer.hpp>
#include <api/keypress_handler.hpp>
#include <api/timer.hpp>
#include <api/resource_locator.hpp>

#include <ui/strided_memcpy.hpp>
#include <ui/ui_types.hpp>
#include <render/helper.hpp>
#include <audio/audiointerface.hpp>

#include <cstdio>
#include <cstdarg>

inline constexpr double framerate_ms = 1000.0 / DFPS;

static void print_chars();
static int esprintf(char* string, const char* format, ...);
static void render_init(api::Framebuffer<modeASCII>& fb, api::Coords bc);
void audio_init(audio::XAudioInterface& ai);
bool valid_distance(TPE_Vec3 v);
void buf_print(char* rawbuf, TPE_Vec3 v);
void buf_print(char* rawbuf, const char* str, TPE_Vec3 v);

int test_engine();
int test_ui();

TPE_Unit elevatorHeight;
TPE_Unit ramp[6] = { 1600,0, -500,1400, -700,0 };
TPE_Unit ramp2[6] = { 2000,-5000, 1500,1700, -5000,-500 };

TPE_Vec3 environmentDistance(TPE_Vec3 p, TPE_Unit maxD) {
    // manually created environment to match the 3D model of it
    TPE_ENV_START( TPE_envAABoxInside(p,TPE_vec3(0,2450,-2100),TPE_vec3(12600,5000,10800)),p )
    TPE_ENV_NEXT(  TPE_envAABox(p,TPE_vec3(-5693,0,-6580),TPE_vec3(4307,20000,3420)),p )
    TPE_ENV_NEXT(  TPE_envAABox(p,TPE_vec3(-10000,-1000,-10000),TPE_vec3(11085,2500,9295)),p )
    TPE_ENV_NEXT(  TPE_envAATriPrism(p,TPE_vec3(-5400,0,0),ramp,3000,2), p)
    TPE_ENV_NEXT(  TPE_envAATriPrism(p,TPE_vec3(2076,651,-6780),ramp2,3000,0), p)
    TPE_ENV_NEXT(  TPE_envAABox(p,TPE_vec3(7000,0,-8500),TPE_vec3(3405,2400,3183)),p )
    TPE_ENV_NEXT(  TPE_envSphere(p,TPE_vec3(2521,-100,-3799),1200),p )
    TPE_ENV_NEXT(  TPE_envAABox(p,TPE_vec3(5300,elevatorHeight,-4400),TPE_vec3(1000,elevatorHeight,1000)),p )
    TPE_ENV_NEXT(  TPE_envHalfPlane(p,TPE_vec3(5051,0,1802),TPE_vec3(-255,0,-255)),p )
    TPE_ENV_NEXT(  TPE_envInfiniteCylinder(p,TPE_vec3(320,0,170),TPE_vec3(0,255,0),530),p )
    TPE_ENV_END
}

int jumpCountdown = 0, onGround = 0;
TPE_Unit playerRotation = 0, headAngle = 0, groundDist;
TPE_Vec3 ballRot, ballPreviousPos, playerDirectionVec;

void updateDirection() {
    playerDirectionVec.x = TPE_sin(playerRotation);
    playerDirectionVec.z = TPE_cos(playerRotation);
    playerDirectionVec.y = 0;
}

int main() {
    std::cout << "Press 'Return' to continue..." << std::endl;
    while(not WIN_PRESSED(VK_RETURN)) {
        if(WIN_PRESSED(VK_ESCAPE)) return 0;
    }

    //return test_engine();
    return test_ui();
}

int test_engine() {
    api::Console::initialize();
    api::ResourceLocator::initialize();
    audio::XAudioInterface::initialize();

    TPE::ObjectModel model("models/level_test.obj", TPE::WindingOrder::eCCW);
    model.compile_model();
    model.set_color(0);

    api::Input input;
    api::Timer time, seconds;

    api::KeypressHandlerFactory khf(input);
    const api::KeypressHandler ESCAPE = khf(VK_ESCAPE, api::eSingle);
    const api::KeypressHandler DRAW_FPS = khf('F', api::eSingle);
    const api::KeypressHandler SPRINTING = khf('R', api::eContinuous);
    const api::KeypressHandler FREECAM = khf(VK_TAB, api::eSingle);
    const api::KeypressHandler DEBUG = khf('I', api::eSingle);

    api::Console window({
        .fullscreen = api::on,
        .font_size = { 16,16 },
        .console_title = "Morpheus, Dorpheus, Orpheus, Go eat some walruses"
    });

    api::Framebuffer<modeASCII> framebuffer;
    api::Coords bufcoords = window.get_buffer_coords();
    render_init(framebuffer, bufcoords);
    framebuffer.swap_buffers();

    auto audio_interface = audio::XAudioInterface::create();
    audio_init(audio_interface);
    std::string music_track = "intersong";

    auto screen_coords = (api::dVec2)(window.get_screen_coords() + 1) / (api::dVec2)framebuffer.get_coords();
    auto screen_middle = (window.get_screen_coords() / 2);
    auto buffer_middle = bufcoords / 2;

    helper_init();
    levelModelInit();
    updateDirection();
    api::CursorHider cur {};

    ballRot = { 0,0,0 };
    tpe_ecs->register_env(environmentDistance);

    /* normally player bodies are approximated with capsules -- since we don't
    have these, we'll use a body consisting of two spheres: */
    auto player_body = tpe_ecs->get_player();
    player_body.move_by(1000, 1000, 1500);
    player_body.rotate_by_axis(0, 0, TPE_F / 4);
    player_body->elasticity = 0;
    player_body->friction = 0;
    player_body->flags |= (TPE_BODY_FLAG_NONROTATING | TPE_BODY_FLAG_ALWAYS_ACTIVE);
    groundDist = TPE_JOINT_SIZE(player_body.foot_joint()) + 30;


    // add two interactive bodies:
    TPE_Unit ball_audio_cooldown = 0;
    auto ball_body = tpe_ecs->bind(
            tpe_ecs->add_ball(1000, TPE::immovable));
    ball_body.move_by(-1000, 1000, 0);
    ball_body->elasticity = 400;
    ball_body->friction = 100;
    ball_body->previouslyCollided = true;
    ball_body->bodyCollisionCallback = [&](TPE_Vec3 velocity) {
        static constexpr auto min_vel = 10;
        static constexpr auto down_time = 10;

        TPE_Unit average_vel = (std::abs(velocity.x / 3) + (velocity.y * 2) + std::abs(velocity.z / 3)) / 3;
        if(average_vel > min_vel && not ball_audio_cooldown) {
            float dist = (float)TPE_dist(ballPreviousPos, player_body->joints[1].position) / TPE_F;
            float volume_modifier = LINEAR_FALLOFF(dist, 25.0f);
            float volume = LINEAR_GAIN(average_vel, 35);
            audio_interface.set_volume("thud", volume * volume_modifier);
            audio_interface.start_source("thud");
            ball_audio_cooldown = down_time;
        }
    };
    ballPreviousPos = ball_body->joints[0].position;

    TPE_Unit box_audio_cooldown = 0;
    auto box_body = tpe_ecs->bind(
            tpe_ecs->add_centered_rect(600, 600, 400, 50));
    box_body.move_by(-3000, 1000, 2000);
    box_body->elasticity = 100;
    box_body->friction = 50;
    box_body->previouslyCollided = true;
    box_body->bodyCollisionCallback = [&](TPE_Vec3 velocity) {
        static constexpr auto min_vel = 7;
        static constexpr auto down_time = 10;

        TPE_Unit average_vel = (std::abs(velocity.x) + (velocity.y * 2) + std::abs(velocity.z)) / 3;
        if(average_vel > min_vel && not box_audio_cooldown) {
            float dist = (float)TPE_dist(ballPreviousPos, player_body->joints[1].position) / TPE_F;
            float volume_modifier = LINEAR_FALLOFF(dist, 25.0f);
            float volume = LINEAR_GAIN(average_vel, 35);
            audio_interface.set_volume("box", volume * volume_modifier);
            audio_interface.start_source("box");
            box_audio_cooldown = down_time;
        }
    };

    // TODO: Add Zero-G
    bool zero_gs = false;
    TPE_Unit gravity_value = 5;

    auto dur = api::ms_duration(framerate_ms);
    double elapsed_time = framerate_ms;
    double average_fps = DFPS;
    bool disp_fps = false;
    bool freecam = true;
    bool debug_draw = false;
    float music_volume = 0.05f;

    auto tick_position = [&] {
        if (onGround) {
            if(input.get_key_held(VK_SPACE) && jumpCountdown == 0) {
                player_body.foot_velocity()[1] = 120;
                jumpCountdown = 8;
            }
        }

        static TPE_Unit D = 15, max_v = 60;
        auto& player_velocity = player_body.foot_velocity();

        if(SPRINTING()) {
            D = 10, max_v = 110;
        }
        else {
            D = 15, max_v = 90;
        }

        if(input.get_key_held('W')) {
            player_velocity[0] += playerDirectionVec.x / D;
            player_velocity[2] += playerDirectionVec.z / D;
        }
        else if(input.get_key_held('S')) {
            player_velocity[0] -= playerDirectionVec.x / D;
            player_velocity[2] -= playerDirectionVec.z / D;
        }

        if (input.get_key_held('A')) {
            player_velocity[2] += playerDirectionVec.x / D;
            player_velocity[0] -= playerDirectionVec.z / D;
        }
        else if (input.get_key_held('D')) {
            player_velocity[2] -= playerDirectionVec.x / D;
            player_velocity[0] += playerDirectionVec.z / D;
        }

        player_velocity[0] = TPE_keepInRange(player_velocity[0], -max_v, max_v);
        player_velocity[2] = TPE_keepInRange(player_velocity[2], -max_v, max_v);
    };
    auto tick_freecam = [&] {
        static constexpr TPE_Unit F = 5;
        static constexpr TPE_Unit D = 2;

        auto& foot_position = player_body.foot_joint().position;
        auto& head_position = player_body.head_joint().position;

        if(input.get_key_held(VK_SPACE)) {
            head_position.y += TPE_F / F;
            foot_position.y += TPE_F / F;
        }
        else if(input.get_key_held(VK_SHIFT)) {
            head_position.y -= TPE_F / F;
            foot_position.y -= TPE_F / F;
        }

        if(input.get_key_held('W')) {
            foot_position.x += playerDirectionVec.x / D;
            foot_position.z += playerDirectionVec.z / D;
        }
        else if(input.get_key_held('S')) {
            foot_position.x -= playerDirectionVec.x / D;
            foot_position.z -= playerDirectionVec.z / D;
        }

        if (input.get_key_held('A')) {
            foot_position.z += playerDirectionVec.x / D;
            foot_position.x -= playerDirectionVec.z / D;
        }
        else if (input.get_key_held('D')) {
            foot_position.z -= playerDirectionVec.x / D;
            foot_position.x += playerDirectionVec.z / D;
        }

        head_position.x = foot_position.x;
        head_position.z = foot_position.z;
    };
    auto tick_draw = [&] {
        if(debug_draw) {
            helper_debugDraw();
            return;
        }

        // Draw functions
        TPE_Unit scale = 1200;
        TPE::draw_model(model, {0,0,0}, {scale,scale,scale}, {0,0,0});

        helper_set3DColor(0); {
        //TPE_Unit scale = 600;
        //helper_drawModel(&levelModel, TPE_vec3(0,0,0), TPE_vec3(scale,scale,scale), TPE_vec3(0,0,0));
    }

        // White color
        /*
        helper_set3DColor(1); {
            helper_draw3DBox(TPE_vec3(5300,elevatorHeight,-4400),
                             TPE_vec3(2000,2 * elevatorHeight,2000),TPE_vec3(0,0,0));

            helper_draw3DBox(box_body.get_center_of_mass(),
                             TPE_vec3(1200,800,1200),
                             box_body.get_rotation(0, 2, 1));

            helper_draw3DSphere(ball_body->joints[0].position,
                                TPE_vec3(1000,1000,1000),ballRot);
        }
         */
    };

    auto poll_volume = [&] {
        static constexpr float max_volume = 2.0f;
        static constexpr float min_volume = 0.0f;
        if(input.get_key_held('K')) {
            if(music_volume > min_volume) {
                music_volume -= 0.02f;
                audio_interface.set_volume(music_track, music_volume);
            }
            else music_volume = min_volume;
        }
        else if(input.get_key_held('L')) {
            if(music_volume < max_volume) {
                music_volume += 0.02f;
                audio_interface.set_volume(music_track, music_volume);
            }
            else music_volume = max_volume;
        }
    };
    auto poll_fps = [&] {
        double measurement = ((double)helper_framev / seconds.elapsed_ms()) * 1000.0;
        if(measurement > 5000.0) measurement = DFPS;

        if(helper_framev > 30) {
            double smoothing = 0.5;
            average_fps = (measurement * smoothing) + (average_fps * (1.0 - smoothing));
            helper_framev = 0;
            seconds.restart();
        }

        if(DRAW_FPS()) disp_fps = !disp_fps;
        if(disp_fps) {
            auto* rawbuf = framebuffer.get_active_buffer()->raw_data();
            esprintf(rawbuf, "FPS: %.1f, Music volume: %i%%    ", average_fps, (int)(music_volume * 100));
            rawbuf += framebuffer->get_x();
            esprintf(rawbuf, "NOW PLAYING: %s   ", music_track.c_str());
            rawbuf += framebuffer->get_x();
            auto pos = player_body->joints[0].position;
            buf_print(rawbuf, "POS", pos);
            rawbuf += framebuffer->get_x();
            esprintf(rawbuf, "X: %.2f, Y: %.1f, Z: %.2f      ",
                     (double)pos.x / TPE_F, (double)pos.y / TPE_F, (double)pos.z / TPE_F);


            auto look_vec = TPE_vec3(playerDirectionVec.x, headAngle, playerDirectionVec.z);
            TPE_Vec3 env_dist = TPE_castEnvironmentRay(
                    player_body->joints[1].position, look_vec,
                    tpe_ecs->get_env(), 128, 512, 128);

            rawbuf += framebuffer->get_x();
            if(valid_distance(env_dist)) {
                buf_print(rawbuf, "LOOKING AT", env_dist);
                //helper_drawPoint3D(env_dist, 1);
            }
            else esprintf(rawbuf, "LOOKING AT: NULL   ");

            rawbuf += framebuffer->get_x();
            esprintf(rawbuf, "GRAVITY: %i   ", gravity_value);
        }
    };
    auto poll_cursor = [&] {
        static auto sensitivity = 4;
        const auto moved_by = (input.get_screen_position() - screen_middle) / sensitivity;
        playerRotation += moved_by.x;
        headAngle -= moved_by.y;
        input.set_screen_position(screen_middle);
    };
    auto poll_sleep = [&] {
        time.stop();
        elapsed_time = framerate_ms - time.elapsed_ms();
        if(elapsed_time > 0.0) {
            dur = api::ms_duration(elapsed_time);
            std::this_thread::sleep_for(dur);
        }
        time.restart();
    };

    if(freecam) player_body->flags ^= TPE_BODY_FLAG_DISABLED;

    audio_interface.set_volume(music_track, music_volume);
    audio_interface.start_source(music_track);
    time.start(), seconds.start();
    while(helper_running) {
        TAG_FRAME("main loop")
        helper_frameStart();
        poll_cursor();
        tpe_ecs->tick();

        if(ball_audio_cooldown > 0) --ball_audio_cooldown;
        if(box_audio_cooldown > 0) --box_audio_cooldown;
        if(jumpCountdown > 0) --jumpCountdown;

        TPE_Vec3 groundPoint =
                environmentDistance(player_body.foot_position(), groundDist);

        onGround = (player_body->flags & TPE_BODY_FLAG_DEACTIVATED) ||
                   (TPE_DISTANCE(player_body.foot_position(),groundPoint)
                    <= groundDist && groundPoint.y < player_body.foot_position().y - groundDist / 2);

        if(not onGround) {
            /* it's possible that the closest point is e.g. was a perpend wall so also
               additionally check directly below */

            onGround = TPE_DISTANCE( player_body.foot_joint().position,
                                     TPE_castEnvironmentRay(
                                             player_body.foot_position(),
                                             TPE_vec3(0,-1 * TPE_F,0), tpe_ecs->get_env(),
                                             128,512,512)) <= groundDist;
        }

        elevatorHeight = (1250 * (TPE_sin(helper_frame * 4) + TPE_F)) / (2 * TPE_F);

        s3l_scene.camera.transform.translation.x = player_body.foot_position().x;
        s3l_scene.camera.transform.translation.z = player_body.foot_position().z;
        s3l_scene.camera.transform.translation.y = TPE_keepInRange(
                s3l_scene.camera.transform.translation.y,
                player_body.head_position().y,
                player_body.head_position().y + 10);

        player_body.multiply_net_speed(onGround ? 300 : 500);

        static constexpr TPE_Unit max_headAngle = TPE_FRACTIONS_PER_UNIT / 4;
        headAngle = TPE_keepInRange(headAngle, -max_headAngle, max_headAngle);
        s3l_scene.camera.transform.rotation.x = headAngle;
        s3l_scene.camera.transform.rotation.y = -1 * playerRotation;

        // fake the sphere rotation (since a single joint doesn't rotate itself):
        TPE_Vec3 ballRoll = TPE_fakeSphereRotation(ballPreviousPos,
                                                   ball_body->joints[0].position,1000);

        ballRot = TPE_rotationRotateByAxis(ballRot,ballRoll);

        ballPreviousPos = ball_body->joints[0].position;

        if(FREECAM()) {
            freecam = !freecam;
            player_body->flags ^= TPE_BODY_FLAG_DISABLED;
        }
        if(freecam) tick_freecam();
        else tick_position();

        updateDirection();

        if(DEBUG()) debug_draw = !debug_draw;
        tick_draw();

        poll_volume();
        poll_fps();
        render::draw_pixel(buffer_middle.x + 1, buffer_middle.y, 3, TO_LUM(15));
        helper_frameEnd();

        if(ESCAPE()) helper_running = false;

        poll_sleep();
    }

    framebuffer.get_active_buffer()->set_buffer_data(255);
    framebuffer.post_buffer();
    window.set_keystate(buffer_middle);
    std::printf("Bye bye!");

    const float fade_rate = music_volume * 0.015;
    while(not audio_interface.fade_out(music_track, fade_rate) and not ESCAPE()) {
        poll_sleep();
    }

    const double exit_time = (music_volume > 0.1) ? 0.5 : (1.5 - music_volume);
    const auto exit_dur = api::ms_duration(exit_time * 400.0);
    std::this_thread::sleep_for(exit_dur);
    return 0;
}


struct Parent {
    Parent() { _ui = new ui::UIFrame<Parent>(this); }
    ~Parent() { delete _ui; }
    ui::UIFrame<Parent>* operator->() { return _ui; }

private:
    ui::UIFrame<Parent>* _ui {};
};

int test_ui() {
    api::Console::initialize();
    api::ResourceLocator::initialize();
    audio::XAudioInterface::initialize();

    TPE::ObjectModel model("models/level_test.obj", TPE::WindingOrder::eCCW);
    model.compile_model();
    model.set_color(0);

    api::Input input;
    api::Timer time, seconds;
    double elapsed_time;
    api::ms_duration dur;

    api::KeypressHandlerFactory khf(input);
    const api::KeypressHandler ESCAPE = khf(VK_ESCAPE, api::eSingle);
    const api::KeypressHandler DEBUG = khf('R', api::eSingle);

    api::Console window({
        .fullscreen = api::on,
        .font_size = { 16,16 },
        .console_title = "Physics Engine"
    });

    api::Framebuffer<modeASCII> framebuffer;
    api::Coords bufcoords = window.get_buffer_coords();
    render_init(framebuffer, bufcoords);

    auto audio_interface = audio::XAudioInterface::create();
    audio_init(audio_interface);
    std::string music_track = "menusong";
    double average_fps = DFPS;
    std::uint32_t frame = 0;

    auto poll_sleep = [&] {
        elapsed_time = framerate_ms - time.elapsed_ms();
        while(elapsed_time > 0.0) {
            elapsed_time = framerate_ms - time.elapsed_ms();
        }
        time.restart();
    };
    auto poll_fps = [&] {
        double measurement = ((double) frame / seconds.elapsed_ms()) * 1000.0;
        if(measurement > 5000.0) measurement = DFPS;

        if(frame > 30) {
            double smoothing = 0.5;
            average_fps = (measurement * smoothing) + (average_fps * (1.0 - smoothing));
            frame = 0;
            seconds.restart();
        }
    };
    auto wipe_screen = [&] {
        framebuffer.swap_buffers();
        framebuffer->set_buffer_data(255);
    };

    Parent p;
    p->bind(input);
    p->bind(window);
    p->bind(framebuffer);

    using ui_type = ui::UIType<Parent>;
    auto& body = p->add_element<ui_type::Element>("body");
    body.align(ui::eCenterX, ui::eCenterY);
    body.set_size(80, 20);

    auto& button = p->add_element<ui_type::Element>("button");
    button.align(ui::eCenterX, ui::eCenterY);
    button.set_size(40, 20);
    button.set_color(178);

    wipe_screen();
    framebuffer.post_buffer();
    p->compile();

    api::Coords buffer_pos = body.position();
    buffer_pos = api::Coords{ -1, 20 };
    body.modify_coords(buffer_pos);
    bool print_data = false;

    audio_interface.set_volume("junglesong", 0.2);
    audio_interface.start_source("junglesong");
    time.start(), seconds.start();
    while(not ESCAPE()) {
        TAG_FRAME("main loop")
        bool modified = false;

        if(input.get_key_held('W')) {
            --buffer_pos.y;
            modified = true;
        }
        if(input.get_key_held('S')) {
            ++buffer_pos.y;
            modified = true;
        }
        if(input.get_key_held('D')) {
            buffer_pos.x += 2;
            modified = true;
        }
        if(input.get_key_held('A')){
            buffer_pos.x -= 2 ;
            modified = true;
        }

        if(modified) {
            body.modify_coords(buffer_pos);
            body.compile();
        }

        auto [cx, cy] = body.position();
        if(DEBUG()) print_data = !print_data;

        BEG_FRAME("rendering")
        wipe_screen();
        p->render();

        if(print_data) {
            auto* rawbuf = framebuffer.get_raw_buffer_data();
            esprintf(rawbuf, "FPS: %.1f    ", average_fps);
            rawbuf += framebuffer->get_x();
            esprintf(rawbuf, "POS: { X: %i, Y: %i }   ", cx, cy);
        }

        framebuffer.post_buffer();
        END_FRAME("rendering")

        poll_fps();
        BEG_FRAME("sleeping")
        poll_sleep();
        ++frame;
        END_FRAME("sleeping")
    }
    return 0;
}

static void render_init(api::Framebuffer<modeASCII>& fb, api::Coords bc) {
    fb.set_details(bc, 3);
    fb.initialize_buffers();
    render::initialize_buffer(fb);
    render::initialize_screen(bc * api::Coords{ 1, 2 });
}

void audio_init(audio::XAudioInterface& ai) {
    ai.register_source("thud", audio::eCircularInstance);
    ai.register_source("box", audio::eCircularInstance);
    ai.register_source("golfcup", audio::eSingleInstance);
    ai.register_source("trophyget", audio::eSingleInstance);

    ai.register_source("bosssong", audio::eLoopingInstance);
    ai.register_source("cavesong", audio::eLoopingInstance);
    ai.register_source("intersong", audio::eLoopingInstance);
    ai.register_source("junglesong", audio::eLoopingInstance);
    ai.register_source("mainsong", audio::eLoopingInstance);
    ai.register_source("menusong", audio::eLoopingInstance);
}

static void print_chars() {
    for(unsigned short c = 0; c < 256; ++c) {
        switch(c) {
            case '\a': {
                std::printf("%i: '\\a'\n", c);
                break;
            }
            case '\b': {
                std::printf("%i: '\\b'\n", c);
                break;
            }
            case '\n': {
                std::printf("%i: '\\n'\n", c);
                break;
            }
            case '\r': {
                std::printf("%i: '\\r'\n", c);
                break;
            }
            case '\t': {
                std::printf("%i: '\\t'\n", c);
                break;
            }
            default: {
                std::printf("%i: '%c'\n", c, c);
            }
        }
    }
    std::cout << std::endl;
}

static int esprintf(char* string, const char* format, ...) {
    va_list ls;
    int ret;

    va_start(ls, format);
    ret = vsprintf(string, format, ls);
    va_end(ls);

    return ret;
}

bool valid_distance(TPE_Vec3 v) {
    return not (v.x == TPE_INFINITY or v.y == TPE_INFINITY or v.z == TPE_INFINITY);
}

void buf_print(char* rawbuf, TPE_Vec3 v) {
    esprintf(rawbuf, "X: %i, Y: %i, Z: %i      ", v.x, v.y, v.z);
}

void buf_print(char* rawbuf, const char* str, TPE_Vec3 v) {
    esprintf(rawbuf, "%s: { X: %i, Y: %i, Z: %i }      ", str, v.x, v.y, v.z);
}