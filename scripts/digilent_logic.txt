// rgData: input, raw digital sample array
// rgValue: output, decoded data array
// rgFlag: output, decoded flag array

const hzNeuro= 10000;

const cSamples = rgData.length // number of acquisition samples
const cSamplePerBit = hzRate/hzNeuro;
var pData, fData = 0;   // previous and current bit value
var cByte = 0;          // byte count

for(var i = 0; i < cSamples ; i++){
    var s = rgData[i];    // current sample
    //s = ~s;               // reverse polarity 
    pData = fData;
    fData = 1&(s>>7);     // pin0 is the data signal
    
    if(pData == 0 && fData != 0){ // rising edge 
        var bValue = 0;
        for(var b = 0; b < 32; b++){
            var ii =round(i+(1.5+b)*cSamplePerBit);
            s = rgData[ii];       // current sample
            //s = ~s;               // reverse polarity 
            fData = 1&(s>>7);     // pin0 is the data signal
            if(fData) bValue |= (1<<b); // serial data bit, LSBit first
        }
        var cWord = cSamplePerBit * 32;
        cByte++;
        for(var j = 0; j < cWord; j++, i++){
            rgFlag[i] = 1;
            rgValue[i] = bValue;
        }
        fData = 0;
    }
}

// value: value sample
// flag: flag sample

function Value2Text(flag, value){
   switch(value){
     case 8454: return "blink";
     default: return value;
    }
}
//Value2Text(1,1)