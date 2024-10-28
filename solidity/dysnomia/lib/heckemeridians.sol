// SPDX-License-Identifier: Sharia
pragma solidity ^0.8.21;
import "../01_dysnomia_v2.sol";

contract Hecke is DYSNOMIA {
    constructor(address ChoAddress) DYSNOMIA("Dysnomia Hecke", "HECKE", address(DYSNOMIA(ChoAddress).Xiao())) {
        _mint(address(this), maxSupply * 10 ** decimals());

        _addMarketRate(AFFECTIONContract, 1 * 10 ** decimals());
        _addMarketRate(ChoAddress, 1 * 10 ** decimals());
    }

    function Compliment(uint256 Waat) public view returns (int256 Longitude, int256 Latitude) {
        assert(Waat <= Meridians[89]);
        uint256 Meridian = GetMeridian(Waat);
        uint256 Start = 0;
        uint256 End;
        bool South = true;

        if(Meridian == 89) {
            South = false;
            Waat = Meridians[89] - Waat;
            Meridian = GetMeridian(Waat);
        }
        
        if(Waat == Meridians[88]) Latitude = 0;
        else Latitude = int256(Meridians[88] - Waat);

        if(Meridian > 0) {
            Start = Meridians[Meridian-1];
            Waat = Waat - Start;
        }

        End = Meridians[Meridian] - Start;

        if(Waat < End/2) {
            Longitude = int256(Waat);
        } else {
            Longitude = int256((Waat - ((End/2) - Start))) * -1;
        }

        Latitude /= 333;
        if(South) Latitude = Latitude * -1;

    }

    function GetWaat(int256 Latitude) public view returns (uint256 Waat) {
        if(Latitude > 0)
            return Meridians[88] + uint256(Latitude * 333);
        return Meridians[88] - uint256(Latitude * -1 * 333);
    }

    function GetMeridian(uint256 Waat) public view returns (uint256 Meridian) {
        for(uint256 i = 0; i < Meridians.length; i++) {
            if(Meridians[i] >= Waat) return i;
        }
    }

uint256[90] public Meridians = [
476733977057179,
3256639860692891,
145031839926114203,
7517342243328022427,
390877483220227250075,
20325604814018987087771,
1056931426015554498647963,
54960434128495401099777947,
2857942574657447424358537115,
148613013882162952633814013851,
7727876721872449223525498804123,
401849589537367335309893107898267,
20896178655943101411801008780793755,
1086601290109041273389339023771359131,
56503267085670146216221315803280758683,
2938169888454847603243484108337769535387,
152784834199652075368661149320131185923995,
7944811378381907919170379740333388838131611,
413130191675859211796859746473022786752927643,
21482769967144679013436706816572871478322321307,
1117104038291523308698708754461765003439930791835,
58089409991159212052332855232011755865443571259291,
3020649319540279026721308472064611280689632875567003,
157073764616094509389508040547359786571547476699568027,
8167835760036914488254418108462708901696155355547621275,
424727459521919553389229741640060862888175765055646390171,
22085827895139816776239946565283164870185115469460782372763,
1148463050547270472364477221394724573249625980098527853467547,
59720078628458064562952815512525677808980550940810015550396315,
3105444088679819357273546406651335246066988648897807375790692251,
161483092611350606578224413145869432795483409742661670108286080923,
8397120815790231542067669483585210505365137306618382532198046291867,
436650282421092040187518813146430946278987139944155867360865577260955,
22705814685896786089750978283614409206507331277096105078451577187653531,
1180702363666632876667050870747949278738381226408997464055168580928067483,
8455205839112304959346349889635311398468931692009966103891402392979584268,
15729709314557977042025648908522673518199482157610934743727636205031101052,
23004212790003649124704947927410035637930032623211903383563870017082617837,
30278716265449321207384246946297397757660583088812872023400103829134134622,
37553219740894993290063545965184759877391133554413840663236337641185651406,
44827723216340665372742844984072121997121684020014809303072571453237168191,
52102226691786337455422144002959484116852234485615777942908805265288684976,
59376730167232009538101443021846846236582784951216746582745039077340201760,
66651233642677681620780742040734208356313335416817715222581272889391718545,
73925737118123353703460041059621570476043885882418683862417506701443235330,
81200240593569025786139340078508932595774436348019652502253740513494752114,
88474744069014697868818639097396294715504986813620621142089974325546268899,
95749247544460369951497938116283656835235537279221589781926208137597785684,
103023751019906042034177237135171018954966087744822558421762441949649302468,
110298254495351714116856536154058381074696638210423527061598675761700819253,
117572757970797386199535835172945743194427188676024495701434909573752336038,
124847261446243058282215134191833105314157739141625464341271143385803852822,
132121764921688730364894433210720467433888289607226432981107377197855369607,
139396268397134402447573732229607829553618840072827401620943611009906886392,
146670771872580074530253031248495191673349390538428370260779844821958403176,
153945275348025746612932330267382553793079941004029338900616078634009919961,
161219778823471418695611629286269915912810491469630307540452312446061436746,
168494282298917090778290928305157278032541041935231276180288546258112953530,
175768785774362762860970227324044640152271592400832244820124780070164470315,
183043289249808434943649526342932002272002142866433213459961013882215987100,
190317792725254107026328825361819364391732693332034182099797247694267503884,
197592296200699779109008124380706726511463243797635150739633481506319020669,
204866799676145451191687423399594088631193794263236119379469715318370537454,
212141303151591123274366722418481450750924344728837088019305949130422054238,
219415806627036795357046021437368812870654895194438056659142182942473571023,
226690310102482467439725320456256174990385445660039025298978416754525087808,
233964813577928139522404619475143537110115996125639993938814650566576604592,
241239317053373811605083918494030899229846546591240962578650884378628121377,
248513820528819483687763217512918261349577097056841931218487118190679638162,
255788324004265155770442516531805623469307647522442899858323352002731154946,
263062827479710827853121815550692985589038197988043868498159585814782671731,
270337330955156499935801114569580347708768748453644837137995819626834188516,
277611834430602172018480413588467709828499298919245805777832053438885705300,
284886337906047844101159712607355071948229849384846774417668287250937222085,
292160841381493516183839011626242434067960399850447743057504521062988738870,
299435344856939188266518310645129796187690950316048711697340754875040255654,
306709848332384860349197609664017158307421500781649680337176988687091772439,
313984351807830532431876908682904520427152051247250648977013222499143289224,
321258855283276204514556207701791882546882601712851617616849456311194806008,
328533358758721876597235506720679244666613152178452586256685690123246322793,
335807862234167548679914805739566606786343702644053554896521923935297839578,
343082365709613220762594104758453968906074253109654523536358157747349356362,
350356869185058892845273403777341331025804803575255492176194391559400873147,
357631372660504564927952702796228693145535354040856460816030625371452389932,
364905876135950237010632001815116055265265904506457429455866859183503906716,
372180379611395909093311300834003417384996454972058398095703092995555423501,
379454883086841581175990599852890779504727005437659366735539326807606940286,
386729386562287253258669898871778141624457555903260335375375560619658457070,
394003890037732925341349197890665503744188106368861304015211794431709973855,
788007780075465850682698395781331007488376212737722608030423588863419947711
];
}