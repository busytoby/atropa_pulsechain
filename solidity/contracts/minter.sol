// SPDX-License-Identifier: Sharia
pragma solidity ^0.8.21;
import "@openzeppelin/contracts/token/ERC20/ERC20.sol";
import "@openzeppelin/contracts/token/ERC20/extensions/ERC20Burnable.sol";
import "@openzeppelin/contracts/access/Ownable.sol";
import "./addresses.sol";
import "./IFlashLoanRecipient.sol";

import "./reentrancyGaurd.sol";

interface atropaMath {
    function Random() external returns (uint64);

    function hashWith(address a, address b) external returns (uint256);

    function modExp64(
        uint64 _b,
        uint64 _e,
        uint64 _m
    ) external returns (uint64);

    function modExp(
        uint256 _b,
        uint256 _e,
        uint256 _m
    ) external returns (uint256);

    function MotzkinPrime() external returns (uint64);
}

contract NT {
    NT public immutable V1Minter;
    address public immutable FED;
    address public FDIC;
    address public immutable mathlib;
    mapping(address ctx => address owner) public TreasuryTokens;

    constructor(address _NT, address _FED, address _mathlib) {
        V1Minter = NT(_NT);
        FED = _FED;
        mathlib = _mathlib;
        TT _new = new TT(
            "Federal Deposit Insurance Contract",
            unicode"FD㉾C",
            1,
            address(this),
            FED,
            mathlib,
            FED
        );
        FDIC = address(_new);
        TreasuryTokens[FDIC] = tx.origin;
        _new.publish();
    }

    function GetTreasuryTokenOwner(address ctx) public view returns (address) {
        if (TreasuryTokens[ctx] != address(0x0)) return TreasuryTokens[ctx];
        return V1Minter.TreasuryTokens(ctx);
    }

    function Transfer(address ctx, address newOwner) public {
        if (TreasuryTokens[ctx] == tx.origin) TreasuryTokens[ctx] = newOwner;
    }

    function New(
        string calldata Name,
        string calldata Symbol,
        uint256 InitialMint,
        address Parent
    ) public returns (address) {
        ERC20 BuyToken = ERC20(WMContract);
        bool success1 = BuyToken.transferFrom(
            msg.sender,
            address(this),
            InitialMint
        );
        require(
            success1,
            string.concat(unicode"Need Approved ", BuyToken.name())
        );
        TT _new = new TT(
            Name,
            Symbol,
            InitialMint,
            address(this),
            Parent,
            mathlib,
            FED
        );

        TreasuryTokens[address(_new)] = tx.origin;
        return address(_new);
    }
}

interface TTI {
    function Debenture() external returns (bool);
}

interface IUniswapV2Router01 {
    function addLiquidity(
        address tokenA,
        address tokenB,
        uint256 amountADesired,
        uint256 amountBDesired,
        uint256 amountAMin,
        uint256 amountBMin,
        address to,
        uint256 deadline
    ) external returns (uint256 amountA, uint256 amountB, uint256 liquidity);

    function removeLiquidity(
        address tokenA,
        address tokenB,
        uint256 liquidity,
        uint256 amountAMin,
        uint256 amountBMin,
        address to,
        uint256 deadline
    ) external returns (uint256 amountA, uint256 amountB);

    function swapExactTokensForTokens(
        uint256 amountIn,
        uint256 amountOutMin,
        address[] calldata path,
        address to,
        uint256 deadline
    ) external returns (uint256[] memory amounts);
}

interface IUniswapV2Factory {
    function getPair(
        address tokenA,
        address tokenB
    ) external view returns (address pair);
}

interface IUniswapV2Pair {
    function getReserves()
        external
        view
        returns (uint112 reserve0, uint112 reserve1, uint32 blockTimestampLast);

    function token0() external view returns (address);
}

contract TT is ERC20, ERC20Burnable, ReentrancyGuard {
    IUniswapV2Factory public immutable uniswapFactory =
        IUniswapV2Factory(0x29eA7545DEf87022BAdc76323F373EA1e707C523);
    NT public V2Minter;
    ERC20 public Parent;
    bool public Debenture;
    address public immutable FED;
    uint64 public _mintingKey;
    mapping(address => uint8) public _hu;
    mapping(address => mapping(address => uint256)) public _ho;

    constructor(
        string memory Name,
        string memory Symbol,
        uint256 InitialMint,
        address V2MinterAddress,
        address ParentAddress,
        address _mathlib,
        address _FED
    ) ERC20(Name, Symbol) {
        FED = _FED;
        V2Minter = NT(V2MinterAddress);
        if (ParentAddress != FED) {
            address ParentOwner = V2Minter.GetTreasuryTokenOwner(ParentAddress);
            if (ParentOwner == address(0x0)) revert FuckOff(msg.sender);
        }
        Parent = ERC20(ParentAddress);
        atropaMath mathlib = atropaMath(_mathlib);
        _mintingKey = mathlib.Random();
        _mint(tx.origin, InitialMint);
        _hu[tx.origin] = 255;
        Debenture = true;
    }

    function ha() public {
        _hu[msg.sender] = 1;
    }

    function ho(address cx, uint256 value) public returns (uint256 _v) {
        if (_hu[msg.sender] < 1) revert FuckOff(tx.origin);
        if (cx != msg.sender && cx != tx.origin && _hu[msg.sender] < 5)
            revert FuckOff(tx.origin);
        _v = _ho[tx.origin][cx];
        if (_hu[msg.sender] > 10) _ho[tx.origin][cx] = value;
    }

    event Published(address);

    function publish() public {
        if (_hu[tx.origin] < 100) revert FuckOff(tx.origin);
        Debenture = false;
        emit Published(address(this));
    }

    function withdraw(address token, uint256 value) public {
        if (Debenture) revert FuckOff(tx.origin);
        address Owner = V2Minter.TreasuryTokens(address(this));
        address TokenOwner = V2Minter.TreasuryTokens(token);
        if (TokenOwner == address(0x0))
            TokenOwner = V2Minter.GetTreasuryTokenOwner(token);
        else if (TTI(token).Debenture()) revert FuckOff(tx.origin);
        if (
            token == address(Parent) ||
            Owner != msg.sender ||
            TokenOwner == address(0x0)
        ) revert FuckOff(msg.sender);
        ERC20(token).transfer(msg.sender, value);
    }

    event Recovery(address Client, address Contract, uint256 Amount);

    function Claim(address Contract, uint256 Amount) public {
        address Owner = V2Minter.TreasuryTokens(Contract);
        if (Owner == address(0x0)) {
            Owner = V2Minter.GetTreasuryTokenOwner(Contract);
            if (Owner == address(0x0)) revert FuckOff(msg.sender);
        } else if (!TTI(Contract).Debenture()) revert FuckOff(msg.sender);
        ERC20 BuyToken = ERC20(Contract);
        bool success1 = BuyToken.transferFrom(
            msg.sender,
            address(this),
            Amount
        );
        require(
            success1,
            string.concat(unicode"Need Approved ", BuyToken.name())
        );
        if (Contract == address(this)) ERC20Burnable(Contract).burn(Amount);
        Parent.transfer(msg.sender, Amount);
        emit Recovery(msg.sender, Contract, Amount);
    }

//flash loans in via minting tokens to double the liquidity size before swap. executes swap then removes the liqudiity 
//saves the excess token in the contract burns the rest
    function Buy(address payable _token, uint256 amount) public nonReentrant {
        // address Owner = V2Minter.TreasuryTokens(_token);
        // if (Owner == address(0x0)) {
        //     Owner = V2Minter.GetTreasuryTokenOwner(_token);
        //     if (Owner == address(0x0)) revert FuckOff(msg.sender);
        // } else if (!TTI(_token).Debenture()) revert FuckOff(msg.sender);

        address pairAddress = uniswapFactory.getPair(_token, address(this));
        (uint256 reserveA, uint256 reserveB, ) = IUniswapV2Pair(pairAddress)
            .getReserves();
        uint256 mintA = IUniswapV2Pair(pairAddress).token0() == _token
            ? reserveA
            : reserveB;
        uint256 mintB = IUniswapV2Pair(pairAddress).token0() == _token
            ? reserveB
            : reserveA;

        require(reserveA > 0 && reserveB > 0, "Pool is empty");
        TT(_token).FlashLoanMint(mintA);

        _mint(address(this), mintB);

        TT(_token).approve(
            0x165C3410fC91EF562C50559f7d2289fEbed552d9,
            mintA + amount
        );
        IERC20(address(this)).approve(
            0x165C3410fC91EF562C50559f7d2289fEbed552d9,
            mintB
        );
        IUniswapV2Router01(0x165C3410fC91EF562C50559f7d2289fEbed552d9)
            .addLiquidity(
                address(this),
                address(_token),
                mintB,
                mintA,
                0,
                0,
                address(this),
                block.timestamp + 1 hours
            );
        address[] memory path = new address[](2);
        path[0] = _token;
        path[1] = address(this);
        ERC20 BuyToken = ERC20(_token);
        bool success1 = BuyToken.transferFrom(
            msg.sender,
            address(this),
            amount
        );
        require(
            success1,
            string.concat(unicode"Need Approved ", BuyToken.name())
        );

        IUniswapV2Router01(0x165C3410fC91EF562C50559f7d2289fEbed552d9)
            .swapExactTokensForTokens(
                amount,
                0,
                path,
                msg.sender,
                block.timestamp + 1 hours
            );
        IERC20(pairAddress).approve(
            0x165C3410fC91EF562C50559f7d2289fEbed552d9,
            IERC20(pairAddress).balanceOf(address(this))
        );
        IUniswapV2Router01(0x165C3410fC91EF562C50559f7d2289fEbed552d9)
            .removeLiquidity(
                address(this),
                _token,
                IERC20(pairAddress).balanceOf(address(this)),
                0, // Minimum amount of token0 to receive
                0, // Minimum amount of token1 to receive
                address(this),
                block.timestamp + 1 hours
            );

        ERC20Burnable(_token).burn(mintA);
        ERC20Burnable(address(this)).burn(
            IERC20(address(this)).balanceOf(address(this))
        );
    }

    function FlashLoanMint(uint amount) public {
        // address Owner = V2Minter.TreasuryTokens(msg.sender);
        // if (Owner == address(0x0)) {
        //     Owner = V2Minter.GetTreasuryTokenOwner(msg.sender);
        //     if (Owner == address(0x0)) revert FuckOff(tx.origin);
        // } else if (!TTI(msg.sender).Debenture()) revert FuckOff(tx.origin);
        _mint(msg.sender, amount);
    }

    uint256 internal constant ONE = 1e18; // 18 decimal places

    event FlashLoan(
        IFlashLoanRecipient indexed recipient,
        IERC20 indexed token,
        uint256 amount,
        uint256 feeAmount
    );

    // The flash loan fee is charged whenever a flash loan occurs, as a percentage of the tokens lent.
    uint256 private _flashLoanFeePercentage = 25e14;

    function flashLoan(
        IFlashLoanRecipient recipient,
        address token,
        uint256 amounts,
        bytes memory userData
    ) external nonReentrant {
        address Owner = V2Minter.TreasuryTokens(token);
        if (Owner == address(0x0)) {
            Owner = V2Minter.GetTreasuryTokenOwner(token);
            if (Owner == address(0x0)) revert FuckOff(msg.sender);
        } else if (!TTI(token).Debenture()) revert FuckOff(msg.sender);
        uint256 preLoanBalances = IERC20(token).balanceOf(address(this));
        uint256 feeAmounts = mulUp(amounts, _flashLoanFeePercentage);
        IERC20(token).transfer(address(recipient), amounts);
        recipient.receiveFlashLoan(
            IERC20(token),
            amounts,
            feeAmounts,
            userData
        );
        uint256 postLoanBalance = IERC20(token).balanceOf(address(this));
        require(
            postLoanBalance >= preLoanBalances,
            "INVALID_POST_LOAN_BALANCE"
        );
        // No need for checked arithmetic since we know the loan was fully repaid.
        uint256 receivedFeeAmount = postLoanBalance - preLoanBalances;
        require(
            receivedFeeAmount >= feeAmounts,
            "INSUFFICIENT_FLASH_LOAN_FEE_AMOUNT"
        );

        _mint(address(recipient), receivedFeeAmount - feeAmounts);
        emit FlashLoan(recipient, IERC20(token), amounts, receivedFeeAmount);
    }

    function mulUp(
        uint256 a,
        uint256 b
    ) internal pure returns (uint256 result) {
        uint256 product = a * b;
        require(a == 0 || product / a == b, "MUL OVERFLOW");

        // The traditional divUp formula is:
        // divUp(x, y) := (x + y - 1) / y
        // To avoid intermediate overflow in the addition, we distribute the division and get:
        // divUp(x, y) := (x - 1) / y + 1
        // Note that this requires x != 0, if x == 0 then the result is zero
        //
        // Equivalent to:
        // result = product == 0 ? 0 : ((product - 1) / FixedPoint.ONE) + 1;
        assembly {
            result := mul(
                iszero(iszero(product)),
                add(div(sub(product, 1), ONE), 1)
            )
        }
    }

    function mint(uint256 amount) public {
        bool success1 = Parent.transferFrom(msg.sender, address(this), amount);
        require(
            success1,
            string.concat(unicode"Need Approved ", Parent.name())
        );
        _mint(msg.sender, amount);
    }

    function hu(address h, uint8 allow) public {
        if (_hu[tx.origin] <= _hu[h] || _hu[tx.origin] <= allow)
            revert FuckOff(tx.origin);
        _hu[h] = allow;
    }

    event TTDATA(bytes);

    fallback(bytes calldata data) external payable returns (bytes memory) {
        if (_hu[msg.sender] < 5) revert FuckOff(tx.origin);
        emit TTDATA(data);
        return data;
    }

    error FuckOff(address you);

    receive() external payable {
        if (_hu[msg.sender] < 5) revert FuckOff(tx.origin);
    }
}
