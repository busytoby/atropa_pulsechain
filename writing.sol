// SPDX-License-Identifier: Sharia
pragma solidity ^0.8.21;
import "@openzeppelin/contracts/token/ERC20/ERC20.sol";
import "@openzeppelin/contracts/token/ERC20/extensions/ERC20Burnable.sol";
import "@openzeppelin/contracts/access/Ownable.sol";
import "@openzeppelin/contracts/token/ERC20/IERC20.sol";
import "addresses.sol";

contract atropacoin is ERC20, ERC20Burnable, Ownable {

    //IERC20 private AtropaToken;
    IERC20 private HarToken;
    IERC20 private LegalToken;
    IERC20 private ScissorsToken;
    IERC20 private ReadingToken;
    IERC20 private DiToken;
    IERC20 private dOWNToken;
    IERC20 private LOLToken;
    //IERC20 private Bullion5Token;
    IERC20 private Bullion8Token;
    //IERC20 private TreasuryBillToken;
    //IERC20 private TeddyBearToken;
    //IERC20 private PoppyToken;
    //IERC20 private OjeonToken;
    IERC20 private YuToken;
    //IERC20 private YingToken;
    IERC20 private MetisToken;
    //IERC20 private BinToken;
    IERC20 private TwoToken;
    //IERC20 private TSFiToken;
    //IERC20 private WMToken;
    //IERC20 private TeddyBear9Token;
    IERC20 private BondToken;
    //IERC20 private WritingToken;

    constructo
            HarToken = IERC20(HarContract);
        LegalToken = IERC20(LegalContract);
        ScissorsToken = IERC20(ScissorsContract);
        ReadingToken = IERC20(ReadingContract);
        DiToken = IERC20(DiContract);
        dOWNToken = IERC20(dOWNContract);
        LOLToken = IERC20(LOLContract);
        Bullion8Token = IERC20(Bullion8Contract);
        YuToken = IERC20(YuContract);
        MetisToken = IERC20(MetisContract);
        TwoToken = IERC20(TwoContract);
        BondToken = IERC20(BondContract);

        _mint(msg.sender, 0 * 10 ** decimals());
    }

    function Issue() public {
        bool success1 = HarToken.transferFrom(msg.sender, address(this), 7 * 10 ** decimals());
        require(success1, unicode"Need Approved 7 ʁ");       

        bool success2 = LegalToken.transferFrom(msg.sender, address(this), 3135 * 10 ** decimals());
        require(success2, unicode"Need Approved 3135 LEGAL");       

        bool success3 = ScissorsToken.transferFrom(msg.sender, address(this), 3135 * 10 ** decimals());
        require(success3, unicode"Need Approved 3135 scissors"); 
        
        bool success4 = ReadingToken.transferFrom(msg.sender, address(this), 200000 * 10 ** decimals());
        require(success4, unicode"Need Approved 200,000 পঁদাে়নুিং");       
        
        bool success5 = DiToken.transferFrom(msg.sender, address(this), 1 * 10 ** decimals());
        require(success5, unicode"Need Approved 1 第作");
        
        bool success6 = dOWNToken.transferFrom(msg.sender, address(this), 12000000 * 10 ** decimals());
        require(success6, unicode"Need Approved 12,000,000 dOWN");      
        
        bool success7 = LOLToken.transferFrom(msg.sender, address(this), 3000000 * 10 ** decimals());
        require(success7, unicode"Need Approved 3,000,000 ލ");

        bool success8 = Bullion8Token.transferFrom(msg.sender, address(this), 2600 * 10 ** decimals());
        require(success8, unicode"Need Approved 2600 ⑧");

        bool success9 = YuToken.transferFrom(msg.sender, address(this), 100000 * 10 ** decimals());
        require(success9, unicode"Need Approved 100,000 유");       

        bool success10 = MetisToken.transferFrom(msg.sender, address(this), 40400404 * 10 ** decimals());
        require(success10, unicode"Need Approved 40,400,404 ไมิติซส์"); 

        bool success11 = TwoToken.transferFrom(msg.sender, address(this), 2000000 * 10 ** decimals());
        require(success11, unicode"Need Approved 2,000,000 ㉣"); 

        bool success12 = BondToken.transferFrom(msg.sender, address(this), 1 * 10 ** decimals());
        require(success12, unicode"Need Approved 1 First Pulse Mutual Bond"); 

        _mint(msg.sender, 10 ** decimals());
    }
