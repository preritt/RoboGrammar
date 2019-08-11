#include <args.hxx>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <robot_design/control.h>
#include <robot_design/render.h>
#include <robot_design/sim.h>

using namespace robot_design;

int main(int argc, char **argv) {
  args::ArgumentParser parser("Robot design search demo.");
  args::HelpFlag help(parser, "help", "Display this help message",
                      {'h', "help"});
  args::Flag verbose(parser, "verbose", "Enable verbose mode",
                     {'v', "verbose"});

  // Don't show the (overly verbose) message about the '--' flag
  parser.helpParams.showTerminator = false;

  try {
    parser.ParseCLI(argc, argv);
  } catch (const args::Completion &e) {
    std::cout << e.what();
    return 0;
  } catch (const args::Help &) {
    std::cout << parser;
    return 0;
  } catch (const args::ParseError &e) {
    std::cerr << e.what() << std::endl;
    std::cerr << parser;
    return 1;
  } catch (const args::RequiredError &e) {
    std::cerr << e.what() << std::endl;
    std::cerr << parser;
    return 1;
  }

  const Scalar time_step = 1.0 / 240;

  // Create a quadruped robot
  std::shared_ptr<Robot> robot = std::make_shared<Robot>(
      /*link_density=*/1.0,
      /*link_radius=*/0.05,
      /*friction=*/0.9,
      /*motor_kp=*/10.0,
      /*motor_kd=*/0.1);
  robot->links_.emplace_back(
      /*parent=*/-1,
      /*joint_type=*/JointType::FREE,
      /*joint_pos=*/1.0,
      /*joint_rot=*/Quaternion::Identity(),
      /*joint_axis=*/Vector3{0.0, 0.0, 1.0},
      /*length=*/0.4);
  for (Index i = 0; i < 4; ++i) {
    Quaternion leg_rot(Eigen::AngleAxis<Scalar>((i - 0.5) * M_PI / 2, Vector3::UnitY()));
    Quaternion thigh_rot(Eigen::AngleAxis<Scalar>(M_PI / 2, Vector3::UnitZ()));
    Quaternion shin_rot(Eigen::AngleAxis<Scalar>(M_PI / 8, Vector3::UnitZ()));
    robot->links_.emplace_back(
        /*parent=*/0,
        /*joint_type=*/JointType::FIXED,
        /*joint_pos=*/(i < 2) ? 1.0 : 0.0,
        /*joint_rot=*/leg_rot,
        /*joint_axis=*/Vector3{0.0, 1.0, 0.0},
        /*length=*/0.2);
    robot->links_.emplace_back(
        /*parent=*/i * 3 + 1,
        /*joint_type=*/JointType::HINGE,
        /*joint_pos=*/1.0,
        /*joint_rot=*/thigh_rot,
        /*joint_axis=*/Vector3{0.0, 0.0, 1.0},
        /*length=*/0.2);
    robot->links_.emplace_back(
        /*parent=*/i * 3 + 2,
        /*joint_type=*/JointType::HINGE,
        /*joint_pos=*/1.0,
        /*joint_rot=*/shin_rot,
        /*joint_axis=*/Vector3{0.0, 0.0, 1.0},
        /*length=*/0.2);
  }

  // Create a floor
  std::shared_ptr<Prop> floor = std::make_shared<Prop>(
      /*density=*/0.0,  // static
      /*friction=*/0.9,
      /*half_extents=*/Vector3{10.0, 1.0, 10.0});

  // Define a lambda function for making simulation instances
  auto make_sim_fn = [&]() -> std::shared_ptr<Simulation> {
    std::shared_ptr<BulletSimulation> sim = std::make_shared<BulletSimulation>(time_step);
    sim->addProp(floor, Vector3{0.0, -1.0, 0.0}, Quaternion::Identity());
    sim->addRobot(robot, Vector3{0.0, 0.425, 0.0}, Quaternion::Identity());
    return sim;
  };

  // Define an objective function
  auto objective_fn = [&](const Simulation &sim) -> Scalar {
    Index robot_idx = sim.findRobotIndex(*robot);
    Matrix4 base_transform;
    sim.getLinkTransform(robot_idx, 0, base_transform);
    return base_transform(1, 3);  // Height of base above ground
  };

  // Create the "main" simulation
  std::shared_ptr<Simulation> main_sim = make_sim_fn();
  unsigned int thread_count = std::thread::hardware_concurrency();
  MPCController controller(*robot, *main_sim, /*horizon=*/5, /*interval=*/30,
                           make_sim_fn, objective_fn,
                           /*thread_count=*/thread_count);
  GLFWRenderer renderer;

  double sim_time = glfwGetTime();
  while (!renderer.shouldClose()) {
    double current_time = glfwGetTime();
    while (sim_time < current_time) {
      controller.update();
      main_sim->step();
      renderer.update(time_step);
      sim_time += time_step;
    }
    renderer.render(*main_sim);
  }
}
