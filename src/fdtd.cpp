#include "fdtd.hpp"

#include "kernels.hpp"
#include "queues.hpp"

#include <cstdlib>
#include <gem5/m5ops.h>

int fdtd(const Configuration &cfg)
{
	// Simulation parameters
	auto [x, y, z, t] = std::tie(cfg.args[0], cfg.args[1], cfg.args[2], cfg.args[3]);
	bool print = cfg.flags & FLAG_PRT;
	phys::params params(x, y, z, t);

	Hx hx(params, print);
	Hy hy(params, print);
	Hz hz(params, print);
	Ex ex(params, print);
	Ey ey(params, print);
	Ez ez(params, print);

	// Queues
	Queue<Msg> *qhx, *qhy, *qhz, *qex, *qey, *qez;
	if (cfg.flags & FLAG_VTL) {
		qhx = new VLQueue<Msg>();
		qhy = new VLQueue<Msg>();
		qhz = new VLQueue<Msg>();
		qex = new VLQueue<Msg>();
		qey = new VLQueue<Msg>();
		qez = new VLQueue<Msg>();
	} else {
		qhx = new SWQueue<Msg>();
		qhy = new SWQueue<Msg>();
		qhz = new SWQueue<Msg>();
		qex = new SWQueue<Msg>();
		qey = new SWQueue<Msg>();
		qez = new SWQueue<Msg>();
	}

	hx.connect(qhx, qey, qez);
	hy.connect(qhy, qez, qex);
	hz.connect(qhz, qex, qey);
	ex.connect(qex, qhy, qhz);
	ey.connect(qey, qhz, qhx);
	ez.connect(qez, qhx, qhy);

	// Run the thing
	std::thread thx([&] { hx.run(); });
	std::thread thy([&] { hy.run(); });
	std::thread thz([&] { hz.run(); });
	std::thread tex([&] { ex.run(); });
	std::thread tey([&] { ey.run(); });
	std::thread tez([&] { ez.run(); });

	cpu_set_t cpusets[6];
	for (int i = 0; i < 6; i++) {
		CPU_ZERO(&cpusets[i]);
		CPU_SET(0, &cpusets[i]);
	}

	switch (cfg.threads) {
	case 1:
		pthread_setaffinity_np(thx.native_handle(), sizeof(cpu_set_t), &cpusets[0]);
		pthread_setaffinity_np(thy.native_handle(), sizeof(cpu_set_t), &cpusets[0]);
		pthread_setaffinity_np(thz.native_handle(), sizeof(cpu_set_t), &cpusets[0]);
		pthread_setaffinity_np(tex.native_handle(), sizeof(cpu_set_t), &cpusets[0]);
		pthread_setaffinity_np(tey.native_handle(), sizeof(cpu_set_t), &cpusets[0]);
		pthread_setaffinity_np(tez.native_handle(), sizeof(cpu_set_t), &cpusets[0]);
		break;
	case 2:
		pthread_setaffinity_np(thx.native_handle(), sizeof(cpu_set_t), &cpusets[0]);
		pthread_setaffinity_np(thy.native_handle(), sizeof(cpu_set_t), &cpusets[0]);
		pthread_setaffinity_np(thz.native_handle(), sizeof(cpu_set_t), &cpusets[0]);
		pthread_setaffinity_np(tex.native_handle(), sizeof(cpu_set_t), &cpusets[1]);
		pthread_setaffinity_np(tey.native_handle(), sizeof(cpu_set_t), &cpusets[1]);
		pthread_setaffinity_np(tez.native_handle(), sizeof(cpu_set_t), &cpusets[1]);
		break;
	case 3:
		pthread_setaffinity_np(thx.native_handle(), sizeof(cpu_set_t), &cpusets[0]);
		pthread_setaffinity_np(thy.native_handle(), sizeof(cpu_set_t), &cpusets[1]);
		pthread_setaffinity_np(thz.native_handle(), sizeof(cpu_set_t), &cpusets[2]);
		pthread_setaffinity_np(tex.native_handle(), sizeof(cpu_set_t), &cpusets[0]);
		pthread_setaffinity_np(tey.native_handle(), sizeof(cpu_set_t), &cpusets[1]);
		pthread_setaffinity_np(tez.native_handle(), sizeof(cpu_set_t), &cpusets[2]);
		break;
	case 6:
		pthread_setaffinity_np(thx.native_handle(), sizeof(cpu_set_t), &cpusets[0]);
		pthread_setaffinity_np(thy.native_handle(), sizeof(cpu_set_t), &cpusets[1]);
		pthread_setaffinity_np(thz.native_handle(), sizeof(cpu_set_t), &cpusets[2]);
		pthread_setaffinity_np(tex.native_handle(), sizeof(cpu_set_t), &cpusets[3]);
		pthread_setaffinity_np(tey.native_handle(), sizeof(cpu_set_t), &cpusets[4]);
		pthread_setaffinity_np(tez.native_handle(), sizeof(cpu_set_t), &cpusets[5]);
		break;
	}

	// Let's go!
	m5_reset_stats(0, 0);
	global_started = true;
	thx.join();
	thy.join();
	thz.join();
	tex.join();
	tey.join();
	tez.join();
	m5_dump_reset_stats(0, 0);

	return EXIT_SUCCESS;
}
