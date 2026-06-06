// SPDX-License-Identifier: Sharia
pragma solidity ^0.8.21;
// import "@openzeppelin/contracts/utils/cryptography/ECDSA.sol";
import "../../include/user.sol";
import "../../01_dysnomia_v2.sol";
import "../../interfaces/11b_lauinterface.sol";

interface CHOInterface {
    function GetUser() external returns(User memory Alpha);
    function VerifyUserTokenPermissions(address UserToken) external;
    function GetUserTokenAddress(address wallet) external view returns(address UserToken);
}

contract PKMinter {
    string public constant Type = "PKMinter";

    PKMinter public constant TreasuryMinter = PKMinter(0xC7bDAc3e6Bb5eC37041A11328723e9927cCf430B);
    PKMinter public constant FederalMinter = PKMinter(0xc15c5F699Daf5e1135732139f05D2c05b3EF4354);
    PKMinter public constant BureauMinter = PKMinter(0x0b92aD7eD0da6c44Bf71B3FCEe668D1670000Ff5);
    PKMinter public constant IndexMinter = PKMinter(0x0c4F73328dFCECfbecf235C9F78A4494a7EC5ddC);
    PKMinter public constant PersonalMinter = PKMinter(0x394c3D5990cEfC7Be36B82FDB07a7251ACe61cc7);

    CHOInterface public Cho;
    address public immutable MRPK;

    mapping(address ctx => address owner) public TreasuryTokens;

    constructor(address ChoAddress) {
        Cho = CHOInterface(ChoAddress);

        bytes32 nonce = hex"00000000000000000000000000000000000000000000000000038d4ae0aec38f";
        address[] memory signers = new address[](40);
        bytes[] memory signatures = new bytes[](40);
        
        signers[0] = 0x3627A3C9831BfbA0e309b791bcf8b4a67a66d14a;                                                                                                 
        signers[1] = 0xd33620a90d87b5FE5AfA2D62Da89EAd8cc6522f4;                                                                                                 
        signers[2] = 0x16baE10A55C51a2b545A5e3fE03b08815db9c9b6;                                                                                                 
        signers[3] = 0xeA3A8F2bb29B0af5bb431d51C7032cd20671FE68;                                                                                                 
        signers[4] = 0xe878b0bfbC4DCeC50f578A709F2Da5c930Ca2312;                                                                                                 
        signers[5] = 0xD61FAef6D82BA65b30FA33fF6C950A6a0c91Ad27;                                                                                                 
        signers[6] = 0x76175171006E564F05e623b6Ff971FF7fFA1C19F;                                                                                                 
        signers[7] = 0xb6521568B61fB1a0f234A96AF72e1243A9177e14;                                                                                                 
        signers[8] = 0xA3D10EC1B4793140bAf7408F371e43E3bb2926A6;                                                                                                 
        signers[9] = 0x02a444A0D5A3E0FA10dead61cC22209cc3b7F07C;                                                                                                 
        signers[10] = 0x2eeed6FDe1f67AcDf9A3ccB54d54DE7d15e109D6;                                                                                                
        signers[11] = 0xEacD7E8D7Cd38B235477a89D0128F7b06D6f8062;
        signers[12] = 0x572aB88D1F50f44C9C429Fa537b3e8885fF2D6dB;
        signers[13] = 0x7B009FF28EE8b39F6D89Ec2EfB8b48C13057e836;
        signers[14] = 0x6dEDf37DAEdDdb7eb1FA810FB3dA4e32F9F699fC;
        signers[15] = 0x0e406020c8fcfF25376F091C16Fed1beAA00D507;
        signers[16] = 0x3645041Fb8c5d01Cd41Ed529431a34A875Aa245b;
        signers[17] = 0x0b38240B25BD2f55455ab57Eb29B83f5E3f2fD2D;
        signers[18] = 0x459b88805CF666bAaA34eDb66116774522827459;
        signers[19] = 0x4Be36205995A2B7df1A6c49d35c7a4252FBc63Af;
        signers[20] = 0x30F8930790C6F4d710AE658366FeC5251ECE93A6;
        signers[21] = 0xCB5CD2aD7c1844B39C2976499E7D8D51c3dfF62D;
        signers[22] = 0x9676c1A8f0C15A470663F49978857a4D0C4cDE9b;
        signers[23] = 0x564B0395591d66b1BB5E5aF4FFCa5b317dF0b574;
        signers[24] = 0x7ab60559f5feDf1b3aB74373B1f1905d458cf61f;
        signers[25] = 0x95B1D678678210569d1f4047A57E1C0b2dE08516;
        signers[26] = 0x7a5db9C9e5469C5d87015404d20DCFe5c97ECea4;
        signers[27] = 0x4C6FfB6BB350fAa7DFD7b4BCc95aBBE06B4cC860;
        signers[28] = 0x31B1BEf75eF21aeE9449A38c88Bc791a5A46A9CE;
        signers[29] = 0xd51a626fFdf82D6F0F79AD39B116371456f54A25;
        signers[30] = 0xF9A4bA57B7a73FBc2bd7eb2BdE76748a622499c7;
        signers[31] = 0x5238fC95024E88b6Ab52dEFaf7d991650D8A328B;
        signers[32] = 0x33CE2dc1B2Dc63662678aC9348abb8E8273795A5;
        signers[33] = 0x2d1AaF46A38772eE341055d04BF7a3A179555490;
        signers[34] = 0x36F057683C583Eb10E98325cd410a277134d9FFa;
        signers[35] = 0x8e63A3a2034e2B20048f6419Cf71A5940Cc2Ff64;
        signers[36] = 0xeB9045500cDD2678b785B1d37656317B67B9Fc82;
        signers[37] = 0xC5c140CE2619C50c44d8CdFCB362454939187483;
        signers[38] = 0x181839E02c3754824F57D03cF17fcAC298f9076F;
        signers[39] = 0x181cF30F1b4F63B6C201169166df7CC8076BC085;   

        signatures[0] = hex"1ce9762ebfbd6996aa494531bc35cfd6332a981f9544945664f5e92f2de195b508562c84e349c82ad81e7e5b5fd75b869991f3a42a360dfc59fe651dee17da701b";
        signatures[1] = hex"cfcadf078d940608bd4a6291f19d9285e10a23fba7a5649c4e7ff9ec71ff479a6bc10117da4111bfb1f9bf0797d85469fd21126d7670b1dd4b6c5344f880abcc1c";
        signatures[2] = hex"56d1f843d055a0998e8e2ac3592e7a20caa119a3896d165fb2b3fc5069a3359e713ed6da1e6fcd8ac07f3e0778c285fc3b323434568c080d17a6a65322f3001c1c";
        signatures[3] = hex"2934a15a9dd50e665c8caa2ec054a0af88a09e48f8118260c8ba57da519e8f0179bf33c378e38f16a12a1e9a2b0f0c3a73e02811846497d665eb922d72d58b771c";
        signatures[4] = hex"977c5e066ee5579e57a952a1e5deb96c622c6337232a80f1d438e9302620b3ee2508448d7b2b3547ecbdb231fca42fb3bb81bfe47fce03ceb0fba6e14e809ba91c";
        signatures[5] = hex"4116cc8aebc94193ac4fa942d439ca1c8b57f0d6df32258df1400ec9fd6b68ef4a5560662e4f27031c39392a1d38bfccc55fbac330e6b591bc0dad66ad38a0ff1b";
        signatures[6] = hex"854bf669145b55a94fda747795970614885980bebcc127cda79d60d6f93425c603412c3e60c5b73761366b16d84544950bc913f230afc1d791b1950ef1a20fd01c";
        signatures[7] = hex"6168419858589558623b4821f5b9c4e5ad6654256303733c6d350902cb2ae23968fd233d59acda93a22c0cb42b49133a85a2f21ef41aaf5fdf423fee99411c391c";
        signatures[8] = hex"1a60172620996f103e4afb39d2e5ba241c23cf78979b2bcfd09a26810398e47d044eacfc8fac5e9833c4d540e4aaa2436ae5ebc5187def83781bd1d10048bc4f1b";
        signatures[9] = hex"786de4e5cd13625e37d68ded72fe5fede403366e196b4880b68b0975be0d75316703ab5c83270bb53e556254dcb0c9ceaf867f078b733f1afcbd98257aba83101b";
        signatures[10] = hex"f97ceff84799c375019d7eb829a3c120ebfeb2e6f83b4ea3501ee405064fac6d464b6ae5e5c18af0e8dc7bfa5e23fdc107ccec8885ecd5cc8bbf1776463fba681b";
        signatures[11] = hex"d127b6ace64f9eb3dd4ad9d76f13fd160a8ad28685f8685f07f0cc4e6af426ee5ddc17f3d063e537a1f227621a72a5de76eb62a7a3832e86c5b77c71a746fd7f1b";
        signatures[12] = hex"0dfc6231618baea0ff3e8c97f9ee40997420c27af56de8e952aaf43cfd956f756911878a029de8220968abb14d5c58e464a233ed1021ef0872f92399f4e3afc21b";
        signatures[13] = hex"d8e546b91368f8552479bb8f8add68b046e93426d4eaa4d7e696d53bbebe1d0356112e9c8050faff78244ca04661543d440eced42da3f9c007cad2f5ac7e3cc41c";
        signatures[14] = hex"7bb2eb21501c9b5175cdea9d51846d1d6423e683562eb83e508c46bfa27b8be87b88228ede4711abe84a8e9f82bca74aa1adc5139ce4e91e8bc1c6ce342d02b91c";
        signatures[15] = hex"824c82ae32473d0a67f167728b2baa34fe3c04fc53d536e5c4d3cb809ff6ac5147658f602e383f0b98c4e89bf2c1330248ff4b3fe7dca6338906aaa6150f67701b";
        signatures[16] = hex"5c0cf6805e51f70fc25f39dd253bd6a795c9edf4b60031ff73f09cb5798550d139598dc836a386cb31156af1b5c29397941c9a16ab7ae0efd4b34467ae48c8f31b";
        signatures[17] = hex"6185413eded1c8ed5e689bbe17b542b31f460759e9cf208631e9ecc4dc153e5f49a4bf9bad5e4754871243bbf533ba17f6a9d05c766cc061f27f8e455a07d8761b";
        signatures[18] = hex"ca99047c96a94736f05dbd19f6216d0a00d5753a753afc007ca5eaf6dbcf8cfd0b548fb22c6dd80fc7beeb314bd7608c4ca06fc7629bd5c1f1e50cd823fb05581b";
        signatures[19] = hex"729f56cf6874330d68c2ab3f9fff0edbbccec17839d2ee6f273405d404a95da33154834c4c7511d70f05fdd78120127c512b8a0abcc2d095d08bd7cc95a3a0531b";
        signatures[20] = hex"6317252a2f8ad963b54eebd12a0b66241b05e3d3e7d873561042c9f6935669a6355686460cea37218456cf47625ec827b6dd8cd8ca186c7c6c586b6cd647fadd1c";
        signatures[21] = hex"84588f6c508f26bc46bbbf7d916656a93d1ad8b7e08e48e0cf90199a6d2a052e5451c2050131023f0c281bdd8d340cceb9a87f66b09d3fe16891786a3a3875821b";
        signatures[22] = hex"eba5e2b9ea4807c474082d32f31aa8c5e8b9ec77c29e95ac603b7693229da17b762912eb4d3cecff9636929c223fee9c5a513ec98713e15c6a26bc1ba77355971b";
        signatures[23] = hex"18e92a81dcaf94ebe217c96ca729403897725150fb6cb2afbb4649692ca69bc5671778f898c21a3d5d4394d49a8156051fd4a93c28d8316ba5b9f2c866be116f1c";
        signatures[24] = hex"89d79c12a28ce0bc39eea2adf138efd630ec07210703bcf500672aa2766b94ab41de5671a57c1ba7c8c66a5813aeca5518df4f6bc29d3f4d425b4fa46d4ab8bb1c";
        signatures[25] = hex"a033daf6dcb5106d9a46096e9566d59240072cb5868ea19d2a7f6e52a9e28c11161d45e66354755c90584008dc9b564cfd7b02bdb5adc5366dd736febe0d045b1b";
        signatures[26] = hex"255fd39bcc28815b5f97c9534b7cf63514753696ab5e77ae34c68b309da2d6ae5ac0018529493d7a6863aabe690eeba479ffca6c8327a66fdd8b0341086993821b";
        signatures[27] = hex"d049f5303505eb597d71e30b24600e5a36c378489f7789ff05053d8c054a20852ce54582a61e177cc798d7fd6663f523297ffe95f9d84dc2b1fb9c6b96d5801e1c";
        signatures[28] = hex"14d00435f1d02fa5fccf80d1832c2b55a1dce730216d4bf84acb014cf50979f029cead99aa46a265d83eaea284a3e21f382b3c45a0d8ac2db900f5b06640317a1c";
        signatures[29] = hex"49b8b12de81063afc0cf4c19778f18beed8c5369c614163aa4d0906949c99d3848ca6a8f66f9277b3926eb68420bdc1ad682f4c9bb77579a27cd065b3f22bd4b1c";
        signatures[30] = hex"0af2d3dce2dae69ac09f27cc39b51e6de6f6c9aa3b2f7b1374f799729951a0bd3a5a82576d12093f2dcde73241f6212723a51d1b7af15ad3fef15cb84fe0740f1b";
        signatures[31] = hex"787f68c781e7a710990d6e07896ad48349cadba55e5a43c0eb1862d5eec0d9de7de0f04ea8ede73dee74188106867c03b717679168ae23df6ccc12ac58310d701b";
        signatures[32] = hex"75ab96bfcfc9b6eb2e53a24e817aab534e6be45d60b8c1bfb8e771de693481d825d8025935686cd379b0f5e9ad4cd65ea55c1a62fd13500bc790af45244596581c";
        signatures[33] = hex"66f52d13a1115f2d640f764b4f5370eb0c2728d57fa931dd87aa444b6c5684101fa786e1a607c889f4e39eba357dc0259caee2fde8ff873f63be24bc658ee5d11c";
        signatures[34] = hex"01164cd0d614b19ce201b3a71adaf64fc42a7bc565e5a616842834524725438c486078648d818bce9352f6cd2c96212c2bb452c44b79deeff9bb720ff4a49f9a1b";
        signatures[35] = hex"00021cd295d39ac669dacb4874ac0948249b8d06f77be4f811a9ae71f35f40aa4f786bc575ab93c84b777beecb07dcd75f5166ee34f4b15bb2b3e2b83fafc6b61b";
        signatures[36] = hex"c3646a30724cf2004267e262ec002cba81f008694584e80c94c1cf4edaedf1b50870ef47c5b46c8e6a755ebd99915a7c66d3ae9b75d0bd884e40e921c5e9cc271c";
        signatures[37] = hex"f14e1b8c27b4ccf875361a2546867a4613614f03f5daa484ba3e8482af419b7d4057b27d68a729bbc336dbf17af282137dbd8c0a51a287439ddebfff94dd4ef71b";
        signatures[38] = hex"b9f35d2ad8f172dae104e0f20a5ef7ad230dccc63e6905e3c62b9962e88dc8002d9f7c2f3be311a8cae50589f30e04a05b8ac8c0f1bc59cdfae383de755418ba1b";
        signatures[39] = hex"f3143c12085a2e2b26021a4d0b3a22bf7e7ee705cc9c5fc86247889d60b78f7147dce5b9c140d857a9d8a7e6c7abb25c5eed6a179d186a897ca03f51a8a0a7b61c";

        TT _new = new TT("PKI Tester", unicode"㊵", 40, address(Cho), signers, nonce, signatures);
        TreasuryTokens[address(_new)] = tx.origin;
        MRPK = address(_new);
    }

    function GetStandardTokenParent(address ctx) public view returns (address) {
        DYSNOMIA NP = TTI(ctx).Parent();
        while(TreasuryTokens[address(NP)] != address(0x0)) NP = TTI(TreasuryTokens[address(NP)]).Parent();
        return address(NP);
    }

    function GetTreasuryTokenOwner(address ctx) public view returns (address) {
        if(TreasuryTokens[ctx] != address(0x0)) return TreasuryTokens[ctx];
        return IndexMinter.GetTreasuryTokenOwner(ctx);
    }

    function Transfer(address ctx, address newOwner) public {
        if(TreasuryTokens[ctx] == tx.origin)
            TreasuryTokens[ctx] = newOwner;
    }

    function New(string memory Name, string memory Symbol, uint8 Complexity, address[] memory Signers, bytes32 Nonce, bytes[] memory Signatures) public returns (address) {
        TT _new = new TT(Name, Symbol, Complexity, address(Cho), Signers, Nonce, Signatures);

        TreasuryTokens[address(_new)] = tx.origin;

        return address(_new);
    }
}

interface TTI {
    function Parent() external view returns (DYSNOMIA);
    function Creator() external view returns (address);
    function publish() external;
}

contract TT is DYSNOMIA {
    PKMinter public immutable PKIMinter;
    CHOInterface public immutable Cho;
    LAU public Parent;
    address public immutable Creator;
    uint256 public immutable Mint;
    address public constant _mathlib = address(0xB680F0cc810317933F234f67EB6A9E923407f05D);
    mapping(address => uint8) public _hu;
    mapping(address => mapping(address => uint256[])) public _ho;

    address[] public PublicKeys;
    uint8 public immutable Complexity;
    mapping(bytes32 => bool) public UsedNonces;

    constructor(string memory Name, string memory Symbol, uint8 _Complexity, address ChoAddress, address[] memory Signers, bytes32 Nonce, bytes[] memory Signatures) DYSNOMIA(Name, Symbol, _mathlib) {
        assert(_Complexity > 0);
        assert(Signers.length == _Complexity);

        PKIMinter = PKMinter(msg.sender);
        Cho = CHOInterface(ChoAddress);
        address useraddress = Cho.GetUserTokenAddress(tx.origin);
        Cho.VerifyUserTokenPermissions(useraddress);
        Parent = LAU(useraddress);
        Creator = tx.origin;

        Complexity = _Complexity;
        for(uint8 i = 0; i < Complexity; i++) {
            assert(Signers[i] != address(0x0));
            PublicKeys.push(Signers[i]);
        }

        assert(ValidateSignatures(Nonce, Signatures) == true);

        _mint(tx.origin, 1 * decimals());
        _hu[tx.origin] = 255;
        Mint = 1 * decimals();
    }

    function ValidateSignatures(bytes32 nonce, bytes[] memory signatures) public returns (bool) {
        if (signatures.length != Complexity) return false;
        if (UsedNonces[nonce]) return false;
        for (uint8 i = 0; i < Complexity; i++) {
            if (signatures[i].length != 65) return false;
            bytes memory signature = signatures[i];
            bytes32 r;
            bytes32 s;
            uint8 v;
        
            assembly {
                r := mload(add(signature, 32))
                s := mload(add(signature, 64))
                v := byte(0, mload(add(signature, 96)))
            }
        
            if (ecrecover(nonce, v, r, s) != PublicKeys[i]) return false; 
        }

        UsedNonces[nonce] = true;
        _mint(tx.origin, 1 * decimals());
        return true;
    }

    function ha() public {
        _hu[msg.sender] = 1;
    }

    function ho(address cx, uint256 value) public returns (uint256 length) {
        if(_hu[msg.sender] < 1) revert FuckOff(tx.origin);
        if(cx != msg.sender && cx != tx.origin && _hu[msg.sender] < 5) revert FuckOff(tx.origin);
        if(value > 0 && _hu[msg.sender] > 10) _ho[tx.origin][cx].push(value);
        length = _ho[tx.origin][cx].length;
    }

    function withdraw(address token, uint256 value) public {
        address Owner = PKIMinter.TreasuryTokens(address(this));
        if(Owner != msg.sender) revert FuckOff(msg.sender);       
        DYSNOMIA(token).transfer(msg.sender, value);
    }

    function hu(address h, uint8 allow) public {
        if(_hu[tx.origin] <= _hu[h] || _hu[tx.origin] <= allow) revert FuckOff(tx.origin);
        _hu[h] = allow;
    }

    event TTDATA(bytes);
    fallback(bytes calldata data) external payable returns (bytes memory) {
        if(_hu[msg.sender] < 5) revert FuckOff(tx.origin);
        emit TTDATA(data);
        return data;
    }

    error FuckOff(address you);
    receive() external payable {
        if(_hu[msg.sender] < 5) revert FuckOff(tx.origin);
    }
}
