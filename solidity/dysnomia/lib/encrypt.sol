// SPDX-License-Identifier: Sharia
pragma solidity ^0.8.21;
import "../01_dysnomia.sol";
import "../interfaces/05b_zhenginterface.sol";

struct Encryption {
    bytes Data;
    uint256 Expiry;
}

contract ENCRYPT is DYSNOMIA {
    string public constant Type = "LibEncrypt";

    ZHENG public Zheng;
    mapping(uint64 => Encryption) private _encryptions;
    uint64[] private _indexes;

    constructor(address ZhengAddress) DYSNOMIA(unicode"CHATLOG LibCrypt", unicode"LibCrypt", address(DYSNOMIA(ZhengAddress).Xiao())) {
        Zheng = ZHENG(ZhengAddress);
        _mintToCap();
    }

    error NotParty(address from, address to, address party);
    function Encrypt(uint64 From, uint64 to, string memory Key, string memory Data) public returns (uint64 index) {
        return Encrypt(From, to, bytes(Key), bytes(Data));
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
    }

    function Decrypt(uint64 From, uint64 to, uint64 Index, string memory Key) public returns (bytes memory) {
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
    }

    function Decrypt(uint64 From, uint64 to, uint64 Index, bytes memory Key) public returns (bytes memory result) {
        Bao memory _from = Zheng.GetRodByIdx(From);
        Bao memory _to = Zheng.GetRodByIdx(to);
        uint64 _e;

        if(!_to.Shio.owner(msg.sender)) revert NotParty(address(_from.Shio), address(_to.Shio), msg.sender);
        (_e, result) = Crypt(_from, _to, Key, _encryptions[Index].Data);
        if(_encryptions[Index].Expiry < block.timestamp) delete _encryptions[Index];
    }

    function hashKey(bytes memory Key) public returns (uint64 result) {
        return uint64(uint256(keccak256(Key)) % Xiao.MotzkinPrime());
    }
}