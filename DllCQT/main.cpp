#include "ConstantQTransform.h"
#include "main.h"

using namespace Cqt;

void WINAPI aaa() {
    const int hopSize = 256;
    const int octaveNumber = 9;
    const int binsPerOctave = 12;

    const int blockSize = 1024;
    const double samplerate = 48000.;

    std::vector<double> audioInputBlock(blockSize, 0.);
    std::vector<double> audioOutputBlock(blockSize, 0.);

    auto cqt = new ConstantQTransform<binsPerOctave, octaveNumber>();
    //cqt.init(hopSize); // separate hop-sizes for each octave can be initialized using the .init(std::vector<int> octaveHopSizes) overload 
    //cqt.initFs(samplerate, blockSize);

    //cqt.inputBlock(audioInputBlock.data(), blockSize);
    //auto schedule = cqt.getCqtSchedule();
    //for (const auto& s : schedule)
    //{
    //    cqt.cqt(s);
    //    auto cqtDomainBuffer = cqt.getOctaveCqtBuffer(s.octave); // the data could now be manipulated in cqt domain
    //    cqt.icqt(s);
    //}
    //auto cqtAudioBlock = cqt.outputBlock(audioInputBlock.size());
    //for (int i = 0; i < audioInputBlock.size(); i++)
    //{
    //    audioOutputBlock[i] = cqtAudioBlock[i];
    //}
}
