// SPDX-License-Identifier: Sharia
pragma solidity ^0.8.21;

import "../../01_dysnomia_v2.sol";
import "../yue.sol";

interface IFolkloreCPU {
    function poke(uint256 addr, uint256 val) external returns (bool);
    function peek(uint256 addr) external view returns (uint256);
}

interface IZMachine {
    function getObjectProperty(uint256 objId, uint256 propId, address player) external view returns (uint256);
}

contract CABSMarketMachine is DYSNOMIA {
    string public constant Type = "CABS_MARKET_MACHINE";

    // CABS double-entry accounts mapping (Account ID => Balances)
    mapping(uint256 => uint256) public CABSAccounts;

    // Escrow register mappings corresponding to Folklore CPU registers
    struct EscrowTrade {
        address depositor;
        address recipient;
        address rewardToken;
        uint256 amount;
        uint256 zObjId;
        uint256 zPropId;
        uint256 targetValue;
        uint256 lockBlock;
        uint256 timeoutBlocks;
        bool active;
    }

    mapping(uint256 => EscrowTrade) public activeTrades;
    uint256 public nextTradeId = 1;

    address public folkloreCPUAddress;
    address public zmachineAddress;

    event CABSAccountUpdated(uint256 indexed account, uint256 oldBalance, uint256 newBalance);
    event EscrowStaged(uint256 indexed tradeId, address indexed depositor, uint256 amount);
    event EscrowSettled(uint256 indexed tradeId, address indexed recipient, uint256 amount, uint256 refundAmount);
    event EscrowRefunded(uint256 indexed tradeId, address indexed depositor, uint256 amount);

    constructor(
        address folklore,
        address zmachine,
        address xiaoAddress
    ) DYSNOMIA("CABS Market Machine", "CABSMM", xiaoAddress) {
        folkloreCPUAddress = folklore;
        zmachineAddress = zmachine;
        // Bypasses deployment limits in parent abstract constructor if xiaoAddress == address(0)
        _mint(msg.sender, 1111111111 * 10 ** decimals());
        addOwner(tx.origin);
    }

    function setBoundAddresses(address folklore, address zmachine) public onlyOwners {
        folkloreCPUAddress = folklore;
        zmachineAddress = zmachine;
    }

    /**
     * @notice Stages a trade in escrow.
     * Moves tokens from the sender's wallet to Account 1200 (Escrow account)
     */
    function stageEscrow(
        address recipient,
        address rewardToken,
        uint256 amount,
        uint256 zObjId,
        uint256 zPropId,
        uint256 targetValue,
        uint256 timeoutBlocks
    ) external returns (uint256 tradeId) {
        require(amount > 0, "Amount must be positive");
        DYSNOMIA token = DYSNOMIA(rewardToken);
        
        // Transfer to this contract's vault
        bool success = token.transferFrom(msg.sender, address(this), amount);
        require(success, "Token transfer failed");

        tradeId = nextTradeId++;
        activeTrades[tradeId] = EscrowTrade({
            depositor: msg.sender,
            recipient: recipient,
            rewardToken: rewardToken,
            amount: amount,
            zObjId: zObjId,
            zPropId: zPropId,
            targetValue: targetValue,
            lockBlock: block.number,
            timeoutBlocks: timeoutBlocks,
            active: true
        });

        // Double-entry CABS bookkeeping: Shift user balance internally to escrow 1200
        CABSAccounts[1100] += amount; // Track volume
        CABSAccounts[1200] += amount; // Locked in escrow

        emit EscrowStaged(tradeId, msg.sender, amount);

        // Sync state to Folklore CPU registers if bound
        if (folkloreCPUAddress != address(0)) {
            folkloreCPUAddress.call(
                abi.encodeWithSignature("poke(uint256,uint256)", 57344, 1) // CABS_CTRL: DEPOSIT
            );
            folkloreCPUAddress.call(
                abi.encodeWithSignature("poke(uint256,uint256)", 57347, amount) // CABS_VAL
            );
        }
    }

    /**
     * @notice Evaluates target condition of a pending escrow trade via the Z-Machine object property.
     * Implements RF SWR/loss logic to calculate Diyat fees.
     */
    function evaluateAndSettle(uint256 tradeId) external returns (bool) {
        EscrowTrade storage trade = activeTrades[tradeId];
        require(trade.active, "Trade not active or already settled");
        require(zmachineAddress != address(0), "Z-Machine address not bound");

        // Peek the quest completion status on the Z-Machine Object/Property
        uint256 currentVal = IZMachine(zmachineAddress).getObjectProperty(trade.zObjId, trade.zPropId, trade.depositor);
        
        if (currentVal == trade.targetValue) {
            trade.active = false;
            
            // RF Loss math: Calculate SWR mismatch tax (simulation of attenuation/diyat cost)
            uint256 reflectionCoefficient = 10; // Simulated 10% reflected wave
            uint256 diyatFee = (trade.amount * reflectionCoefficient) / 100;
            uint256 payout = trade.amount - diyatFee;

            DYSNOMIA token = DYSNOMIA(trade.rewardToken);
            
            // Execute payouts
            token.transfer(trade.recipient, payout);
            if (diyatFee > 0) {
                token.transfer(tx.origin, diyatFee); // Diyat fees routed to deployer treasury
            }

            // Bookkeeping update
            CABSAccounts[1200] -= trade.amount;
            CABSAccounts[2200] += diyatFee; // Accrued Diyat fees

            emit EscrowSettled(tradeId, trade.recipient, payout, diyatFee);

            if (folkloreCPUAddress != address(0)) {
                folkloreCPUAddress.call(
                    abi.encodeWithSignature("poke(uint256,uint256)", 57344, 2) // CABS_CTRL: COMMIT
                );
            }
            return true;
        }
        return false;
    }

    /**
     * @notice Executes refund (wave reflection) if the trade has timed out without completion.
     */
    function refundEscrow(uint256 tradeId) external {
        EscrowTrade storage trade = activeTrades[tradeId];
        require(trade.active, "Trade not active");
        require(block.number >= trade.lockBlock + trade.timeoutBlocks, "Escrow lock period active");

        trade.active = false;
        DYSNOMIA token = DYSNOMIA(trade.rewardToken);

        // Wave Reflection: Route locked tokens back to depositor
        token.transfer(trade.depositor, trade.amount);

        CABSAccounts[1200] -= trade.amount;

        emit EscrowRefunded(tradeId, trade.depositor, trade.amount);

        if (folkloreCPUAddress != address(0)) {
            folkloreCPUAddress.call(
                abi.encodeWithSignature("poke(uint256,uint256)", 57344, 3) // CABS_CTRL: REFUND
            );
        }
    }
}
