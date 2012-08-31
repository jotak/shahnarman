cost = {0,0,0,2}
icon = "spellstealing"
allowedInBattle = 1

function getName()
	if language == "french" then
		return "Vol de sort"
	else
		return "Spell stealing"
	end
end

function getDescription()
	if language == "french" then
		return "Choisissez un sort dans la main d'un adversaire, et prenez-le en main."
	else
		return "Choose a spell from an opponent's hand and take it in hand."
	end
end

function onCast()
	-- nothing here, because the target selection can be done only during resolve phase (specific to "spell_in_deck" and "spell_in_hand" targets to avoid cheating)
end

_caster = 0
function onResolve(params, caster)
	_caster = caster
	selectTargetThenResolve("spell_in_hand", "opponent", "doResolve")
end

-- this function receives playedId and spellId as parameter, in a space-separated string
function doResolve(params)
	player, spell = split(params, " ")
	changeSpellOwner("spell_in_hand", player, spell, _caster)
end
