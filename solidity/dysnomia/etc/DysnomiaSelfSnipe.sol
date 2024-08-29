pragma solidity ^0.8.2;
import "../../addresses.sol";

//<alexander_as_helios> people can deploy this to chat and purchase their own token
//                      https://gist.github.com/as-helios/cc9bb3679d03eff83b000635c45e0451

interface IERC20 {
    function approve(address _spender, uint256 _value) external returns (bool);
    function balanceOf(address _owner) external view returns (uint256);
    function transferFrom(address _sender, address _recipient, uint256 _value ) external returns (bool);
    function transfer(address _recipient, uint256 _value) external returns (bool);
}

interface ILAU {
    function Chat(string calldata text) external;
    function Purchase(address token, uint256 amount) external;
}

error Unauthorized();
error InsufficientLAUBalance();

contract DSS {
    address ownerAddress;
    address LAUAddress;
    address AffectionAddress = AFFECTIONContract;
    ILAU LAUContract = ILAU(LAUAddress);
    IERC20 LAUContractERC20;
    IERC20 AffectionContract = IERC20(AffectionAddress);

    constructor(address UserToken) {
        LAUAddress = UserToken;
        LAUContractERC20 = IERC20(LAUAddress);
        ownerAddress = msg.sender;
        AffectionContract.approve(LAUAddress, type(uint256).max);
    }

    modifier onlyOwner() {
        if (msg.sender != ownerAddress)
            revert Unauthorized();
        _;
    }

    function setLAU(address tokenAddress) public onlyOwner {
        LAUAddress = tokenAddress;
        AffectionContract.approve(tokenAddress, type(uint256).max);
    }

    function withdrawLAU(uint256 amount) public payable onlyOwner {
        if (LAUContractERC20.balanceOf(address(this)) < amount)
            revert InsufficientLAUBalance();
        LAUContractERC20.transfer(msg.sender, amount);
    }

    function chatAndSnipe(string calldata text) public onlyOwner {
        LAUContract.Chat(text);
        AffectionContract.transferFrom(msg.sender, address(this), 1*10**18);
        LAUContract.Purchase(AffectionAddress, 1*10**18);
        LAUContractERC20.transfer(msg.sender, 1*10**18);
    }

    function chat(string calldata text) public onlyOwner {
        LAUContract.Chat(text);
    }
}