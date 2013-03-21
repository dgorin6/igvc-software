#include "StereoPlayback.h"
#include <boost/thread/condition_variable.hpp>
#include <unistd.h>

StereoPlayback::StereoPlayback(std::string leftVideo, std::string rightVideo, int fps=20) : _leftVid(leftVideo), _rightVid(rightVideo),
    _framesPerSecond(fps), _playbackThread(&StereoPlayback::Run, this)
{
}

void StereoPlayback::Run()
{
    int waitTime = 1000000/_framesPerSecond;
    bool lsuccess, rsuccess;
    while(Running())
    {
        usleep(waitTime); //TODO Ensure this function call is non-blocking(gives up the processor during sleep) or replace it with something that is

        Mat left, right;
        lsuccess = _leftVid.grab();
        _leftVid.retrieve(left);
        rsuccess = _rightVid.grab();
        if (lsuccess && rsuccess)
        {
            _rightVid.retrieve(right);
            _images = StereoPair(left, right);
            onNewData(_images);
        }
        else
        {
            LockRunning();
            Running(false);
            UnlockRunning();
        }
    }
}

StereoPlayback::~StereoPlayback()
{
    //dtor
}
