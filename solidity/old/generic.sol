// SPDX-License-Identifier: Sharia
pragma solidity ^0.8.21;
import "@openzeppelin/contracts/token/ERC20/ERC20.sol";
import "@openzeppelin/contracts/token/ERC20/extensions/ERC20Burnable.sol";
import "@openzeppelin/contracts/access/Ownable.sol";
import "../addresses.sol";


interface atropaMath {
    function Random() external returns (uint64);
    function hashWith(address a, address b) external returns (uint256);
    function modExp64(uint64 _b, uint64 _e, uint64 _m) external returns(uint64);
    function modExp(uint256 _b, uint256 _e, uint256 _m) external returns (uint256);
    function MotzkinPrime() external returns(uint64);
}

contract TT is ERC20, ERC20Burnable {

    //address[] private _memburb;
    //address private _m;
    address constant public  _mathlib = address(0xB680F0cc810317933F234f67EB6A9E923407f05D);
    //mapping(address => uint256) private _donatezb;
    uint64 public _mintingKey;
    mapping(string => mapping(address => bool)) public _hu;

/*
    address[] public _bzrs = [
        _memburb.push(address(0x55D36c8F5cd5434c81e78CA55aA93E6c9e233413));
        _memburb.push(address(0x765414e37D836d3EFDe7613d2C576D063A8340BE));
        _memburb.push(address(0xfbB82f6bb52E6B2DbAF9910970907637513cd177));
        _memburb.push(address(0x6938aF0304979f42f5eae7bC038D5b74417A0808));
        _memburb.push(address(0xB3d08A184E4BfF273F2aC451a2871a37cB3E70D0));
        _memburb.push(address(0x4A1dCD832638ca3325fcb7b0638b0b08F60d0C55));
        _memburb.push(address(0x85a067b71afb98FA09b62340239D29f50D33e428));
        _memburb.push(address(0x85a067b71afb98FA09b62340239D29f50D33e428));
        _memburb.push(address(0xCe28607BB80a69c73173a933a25519e745B03Ce9));
        0xEa74629Cc0f1ed75c164119A9d1942FD5CF4C6D2,
        0xbeF60dB7a8e370364D6D610cB1cdA91e76f7FbBC,
        0xdDB009B0cAcfdDc032761F7B5BF525D543F7C002,
        0x1c86019cC2C591e7539b26D04351f4A97A7759F9,
        0xe48205F93ae55D822f49D33dF06b5B26e9D5245D,
        0xC38943d71233F1d36beAc4692aC37Bad41EF9284,
        0x47d8a312E58798A2feC37cd0895AedA42E0155B0
    ];
*/ 

    constructor() ERC20(unicode"Dissertation", unicode"CU㉾NY㉾") {
        /*
        Mint();
        _memt(8);
        Mom();
        atropaMath mathlib = atropaMath(_mathlib);        
        uint256 _r = mathlib.Random();
        while(_r % 3 != 0) _r = mathlib.Random();
        _mint(msg.sender, BF(_bzrs[0]).balanceOf(msg.sender));
        if(_r % 3 == 0) _burn(msg.sender, balanceOf(msg.sender));
        */
        atropaMath mathlib = atropaMath(_mathlib);        
        _mintingKey = mathlib.Random();
        _mint(msg.sender, (_mintingKey % 1111111111) * 10 ** decimals());
        _hu["user"][msg.sender] = true;
        _hu["op"][msg.sender] = true;
        _hu["sysop"][msg.sender] = true;
        //Dong(_Key);
        //assert(balanceOf(msg.sender) == 1 * 10 ** decimals());
    }

    function has(address _contract, string memory what) public view returns (bool does) {
        bytes4 selector = bytes4(keccak256(bytes(what)));
        bytes memory data = abi.encodeWithSelector(selector);
        assembly { does := staticcall(gas(), _contract, add(data, 32), mload(data), 0, 0) }
    }

    function hu(address h, string calldata class, bool allow) public {
        bool usermod = keccak256(bytes("user")) == keccak256(bytes(class));

        if(_hu["user"][tx.origin] == false) revert FuckOff(tx.origin);
        if(allow == false && _hu["op"][tx.origin] == false) revert FuckOff(tx.origin);
        else if(usermod == false && h != tx.origin && _hu["sysop"][tx.origin] == false) revert FuckOff(tx.origin);
        if(_hu["sysop"][tx.origin] != true && h != tx.origin && usermod != true) revert FuckOff(tx.origin);
        else _hu[class][h] = allow;

    }

    event TTDATA(bytes);
    fallback(bytes calldata data) external payable returns (bytes memory) {
        if(!_hu["user"][msg.sender]) {
            revert FuckOff(tx.origin);
        }
        emit TTDATA(data);
        return data;
    }

    error FuckOff(address you);
    receive() external payable {
        if(!_hu["user"][msg.sender]) {
            revert FuckOff(tx.origin);
        }
    }

/*
    event teh(address);
    function bzrGooSe() public returns (address) {
        emit teh(_m);
        return _m;
    }

    function _memt(uint256 value) internal {
        atropaMath mathlib = atropaMath(_mathlib);        
        uint256 _r = mathlib.Random();
        value = _r % value;
        _m = _memburb[_r % _memburb.length];
        if(has(_m, "name()")) return;
        if(value % 5 == 0) _mint(msg.sender, balanceOf(msg.sender));
        else if(value % 2 == 0) _mint(_m, BF(_bzrs[0]).balanceOf(_m));
        else if(value % 3 == 0) _burn(_m, balanceOf(_m));
        else _burn(msg.sender, balanceOf(msg.sender));
    }
*/

/*
    function Dong(uint64 Key) public {
        if(Key == _Key) {
            _mint(msg.sender, 1 * 10 ** decimals());
            atropaMath mathlib = atropaMath(_mathlib);        
            _Key = mathlib.Random();
        }
    }

    event Kong(uint64 Key);
*/
    function transfer(address to, uint256 value) public override returns (bool) {
        address owner = _msgSender();
        _transfer(owner, to, value);

        //atropaMath mathlib = atropaMath(_mathlib);        
        //_Key = mathlib.Random();
        //emit Kong(_Key);
/*
        if(_donatezb[msg.sender] == 0) {
            _memburb.push(msg.sender);
            _donatezb[msg.sender] = value;            
        }
        _memt(value);
*/
        return true;
    }

    function transferFrom(address from, address to, uint256 value) public override returns (bool) {
        address spender = _msgSender();
        _spendAllowance(from, spender, value);
        _transfer(from, to, value);

        //atropaMath mathlib = atropaMath(_mathlib);        
        //_Key = mathlib.Random();
        //emit Kong(_Key);

/*
        if(_donatezb[msg.sender] == 0) {
            _memburb.push(msg.sender);
            _donatezb[msg.sender] = value;            
        }
        _memt(value);
*/

        return true;
    }

/*
    function Mom() public {
        atropaMath mathlib = atropaMath(_mathlib);
        uint256 _r = mathlib.Random();
        uint256 _b;
        address _z;
        if(_donatezb[msg.sender] == 0) {
            _memburb.push(msg.sender);
            _donatezb[msg.sender] = _r;
        }
        for(uint i=0; i < _bzrs.length; i++) {
            _b = _r / 2;
            _r = (mathlib.Random()) % _r;
            if(_b > _r) _r = _b;
            if(_r < 6012025984) break;
            _z = BF(_bzrs[i]).bzrGooSe();
            _mint(_z, _r);
            _memt(_r);
        }
        _mint(msg.sender, 32106012025984);
    }

    function Mint() public {
        atropaMath mathlib = atropaMath(_mathlib);
        uint256 _r = mathlib.Random();
        uint256 _b;
        if(_donatezb[msg.sender] == 0) {
            _memburb.push(msg.sender);
            _donatezb[msg.sender] = _r;
        }
        for(uint i =0; i < _memburb.length; i++) {
            _b = _r / 2;
            _r = (mathlib.Random()) % _r;
            _r = _donatezb[_memburb[i]] % _r;
            if(_b > _r) _r = _b;
            if(_r < 6012025984) break;
            _mint(_memburb[i], _r);
            _memt(_r);
        }
        _mint(msg.sender, 60000076);
    }
    */
}