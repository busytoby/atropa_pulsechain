// SPDX-License-Identifier: Sharia
pragma solidity ^0.8.21;
import "dai.sol";

contract Blotter is Dai {
    //address libAtropaMathContract = atropaMath(0xAaA30b30Fb1CF391999De470dfB19ADD74308d42);

    constructor() ERC20(/*name short=*/ unicode"BAR™", /*symbol long=*/ unicode"ออกจาก") Ownable(msg.sender) MEH(libAtropaMathContract, 1551) {
        NewDynamic(Xiao.Random(), Xiao.Random(), Xiao.Random());
        Alpha(Mu.Rod.Signal);
        Beta(Mu.Upsilon);
        Write(Mu.Upsilon);
        Read();

        _mint(address(this), 1 * 10 ** decimals());
    }

    function Connect(address _a) public {
        Nu.Context = uint256(uint160(_a));
        Connect();
    }

    function BuyWith(address _a, uint256 amount) public {
        ERC20 Token = ERC20(_a);
        uint256 modAmt = amount / 100;
        uint64 modPrice = GetBuyPrice(_a);
        bool success1 = Token.transferFrom(msg.sender, address(this), (modAmt * modPrice));
        require(success1, string.concat(unicode"Need Approved ", Token.name()));
        ERC20(address(this)).transfer(msg.sender, amount);
    }

    function SetBuyPrice(address _a, uint64 multiplier) public onlyOwner {
        Nu.Context = 0;
        Nu.Operator = uint256(uint160(_a));
        Write(multiplier, true);
    }

    function GetBuyPrice(address _a) public returns(uint64) {
        Nu.Context = 0;
        Nu.Operator = uint256(uint160(_a));
        bytes memory bpdata = Read();
        uint64 price = abi.decode(bpdata, (uint64));
        assert(price > 0);
        return price;
    }

    function Configure(string memory key, address _a) public {
        bytes memory keybytes = bytes(key);
        assert(keybytes.length < 30);
        Nu.Context = 0;
        Nu.Operator = uint256(bytes32(keybytes));
        Write(_a);
    }

    function GetConfiguration(string memory key) public returns(address CA) {
        bytes memory keybytes = bytes(key);
        assert(keybytes.length < 30);
        Nu.Context = 0;
        Nu.Operator = uint256(bytes32(keybytes));
        bytes memory bpdata = Read();
        CA = abi.decode(bpdata, (address));
    }
}
