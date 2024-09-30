// SPDX-License-Identifier: Sharia
pragma solidity ^0.8.21;
import "../../01_dysnomia_v2.sol";
import "../yue.sol";
import "../sky/interfaces/02b_choainterface.sol";

contract SEI is DYSNOMIA {
    string public constant Type = "SEI";

    CHOA public Choa;
    mapping(address => address Chi) private _yan;

    constructor(address ChoaAddress) DYSNOMIA("Dysnomia Sei", "SEI", address(DYSNOMIA(ChoaAddress).Xiao())) {
        Choa = CHOA(ChoaAddress);
        Choa.Chan().Xie().Xia().Mai().Qi().Zuo().VAT().addOwner(address(this));
        addOwner(tx.origin);
        _mintToCap();
    }

    error NotStarted(address);
    function Chi() public view returns (YUE) {
        if(_yan[tx.origin] == address(0x0)) revert NotStarted(tx.origin);
        return YUE(_yan[tx.origin]);
    }

    function Start(string calldata Name, string calldata Symbol) public returns (YUE _chi) {
        if(_yan[tx.origin] == address(0x0)) {
            _chi = new YUE(Name, Symbol, address(Choa.Chan().Xie()));
            _chi.addOwner(address(Choa));
            Choa.Chan().Chou();
            _yan[tx.origin] = address(_chi);
            return _chi;
        }

        _chi = Chi();
        _chi.Rename(Name, Symbol);
        Choa.Chan().Chou();
        return _chi;
    }
}