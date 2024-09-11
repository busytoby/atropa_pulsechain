// SPDX-License-Identifier: Sharia
pragma solidity ^0.8.21;
import "../01_dysnomia.sol";
import "../interfaces/10b_voidinterface.sol";

struct Encryption {
    bytes Data;
    uint256 Expiry;
}

contract ENCRYPT is DYSNOMIA {
    string public constant Type = "LibEncrypt";

    ZHENG public Zheng;
    VOID public Void;
    mapping(uint64 => Encryption) private _encryptions;
    uint64[] private _indexes;

    constructor(address VoidAddress) DYSNOMIA(unicode"CHATLOG LibCrypt", unicode"LibCrypt", address(DYSNOMIA(VoidAddress).Xiao())) {
        Void = VOID(VoidAddress);
        addOwner(VoidAddress);
        Zheng = Void.Nu().Psi().Mu().Tau().Upsilon();
        Void.AddLibrary("encrypt", address(this));
        _mintToCap();
    }

    error NotParty(address from, address to, address party);
    function Encrypt(uint64 From, uint64 to, string memory Key, string memory Data) public returns (uint64 index) {
        _mintToCap();
        return Encrypt(From, to, bytes(Key), bytes(Data));
    }

    function Encapsulate(Bao memory From, uint64 Gamma, uint64 Rho, uint64 Upsilon, uint64 Ohm) public returns (uint64 Entropy, bytes memory Geng) {
        bytes memory Gai = new bytes(8);
        Geng = new bytes(24);

        for(uint i = 0; i < 8; i++)
            Gai[i] = bytes1(uint8((Gamma >> (8*i)) & 255));

        for(uint i = 0; i < 8; i++)
            Geng[i] = bytes1(uint8((Rho >> (8*i)) & 255));
        for(uint i = 8; i < 16; i++)
            Geng[i] = bytes1(uint8((Upsilon >> (8*(i-8))) & 255));
        for(uint i = 16; i < 24; i++)
            Geng[i] = bytes1(uint8((Ohm >> (8*(i-16))) & 255));
      
        (Entropy, Geng) = Crypt(From, From, Gai, Geng);
        _mintToCap();
    }

    error ChromosomeLength(uint8 minimum, uint256 length);
    function Saat(bytes memory Geng) public pure returns (uint64[3] memory Go) {
        if(Geng.length < 18) revert ChromosomeLength(18, Geng.length);
        uint j=0;
        Go[j] = Go[j] + uint8(Geng[0]);
        for(uint i = 1; i < Geng.length; i++){
            Go[j] = Go[j] + uint8(Geng[i]);
            if(i%8 == 0 && j<2) j++;
        }
    }

    function Encrypt(uint64 From, uint64 to, bytes memory Key, bytes memory Data) public returns (uint64 index) {
        Bao memory _from = Zheng.GetRodByIdx(From);
        Bao memory _to = Zheng.GetRodByIdx(to);
        bytes memory result;

        if(!_from.Shio.owner(msg.sender)) revert NotParty(address(_from.Shio), address(_to.Shio), msg.sender);
        (index, result) = Crypt(_from, _to, Key, Data);
        _encryptions[index].Data = result;
        _encryptions[index].Expiry = block.timestamp + (10 days);
        _indexes.push(index);
        _mintToCap();
    }

    error TooManyCrypts(uint64 max, uint256 count);
    function Crypt(Bao memory From, Bao memory to, bytes memory Key, bytes memory Data) internal returns (uint64 indexable, bytes memory result) {
        if(_indexes.length >= Xiao.MotzkinPrime()) revert TooManyCrypts(Xiao.MotzkinPrime(), _indexes.length);

        uint64 _b;
        uint64 _e;
        uint64 _m;

        (_e, _m) = From.Shio.Rho().Rod.React(hashKey(Key), From.Shio.Rho().Cone.View().Channel);
        result = new bytes(Data.length); 

        for(uint256 i = 0; i < Data.length; ) {
            (_b, _m) = to.Shio.Rho().Cone.React(_e, to.Shio.Rho().Rod.View().Channel);
            for(uint256 j = 0; j < 8 && i < Data.length; j++) {
                result[i] = bytes8(_m)[j] ^ Data[i];
                i++;
            }
            (_e, _m) = From.Shio.Rho().Rod.React(_b, From.Shio.Rho().Cone.View().Channel);
        }
        indexable = _e;
        _mintToCap();
    }

    function Decrypt(uint64 From, uint64 to, uint64 Index, string memory Key) public returns (bytes memory) {
        _mintToCap();
        return Decrypt(From, to, Index, bytes(Key));
    }

    function Prune() public {
        for(uint64 i = 0; i < uint64(_indexes.length); i++) {
            if(_encryptions[i].Expiry == 0 || _encryptions[i].Expiry < block.timestamp) {
                delete _encryptions[i];
                _indexes[i] = _indexes[_indexes.length - 1];
                _indexes.pop();
            }
        }
        _mintToCap();
    }

    function Decrypt(uint64 From, uint64 to, uint64 Index, bytes memory Key) public returns (bytes memory result) {
        Bao memory _from = Zheng.GetRodByIdx(From);
        Bao memory _to = Zheng.GetRodByIdx(to);
        uint64 _e;

        if(!_to.Shio.owner(msg.sender)) revert NotParty(address(_from.Shio), address(_to.Shio), msg.sender);
        (_e, result) = Crypt(_from, _to, Key, _encryptions[Index].Data);
        if(_encryptions[Index].Expiry < block.timestamp) delete _encryptions[Index];
        _mintToCap();
    }

    function hashKey(bytes memory Key) public returns (uint64 result) {
        _mintToCap();
        return uint64(uint256(keccak256(Key)) % Xiao.MotzkinPrime());
    }
}